#include "serverFuncs.h"

void freeProcLog(void * procLog) {
	procLogInit *data = procLog;
    //free(data->name) // string é fixa
    free(data);
}

void daemonize(void) {
	pid_t pid, sid;

    // Fork the process
    pid = fork();

    if (pid < 0) {
	// error
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process
        _exit(EXIT_SUCCESS);
    }

    // Set file mode creation mask to 0
	// ??????????????????????????????
    umask(0);

    // Create a new session
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // Change the working directory to root
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

	printf("Server is pid %d\n", getpid());
	fflush(stdout);

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main (int argc, char **argv) {
    // fazer pipe
    if (argc != 2) {
        perror("format should be: ./monitor finish_proc_folder_path");
        exit(1);
    }

	if (mkfifo(PIPE_NAME, 0600) != 0) {
		perror("Error making pipe");
	}

    //criar diretoria de destino de processos, se ainda não existir
    mkdir(argv[1], 0700); // utilizador com permissões read,write e execute

    // Decidimos usar Hash table para gerir processos em execução. 
    // A possiblidade de ter processos com PIDs muito distintos invalidou o uso de um array, porque teria muitos espaços vazios
    // A hashtable usa como key os PIDs porque queries de status usam PID 
    // e porque funções de hash para PIDs (ints) são mais simples do que para nomes de procs (strings)

    GHashTable * live_procs = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, freeProcLog); // free de keys (ints) é NULL

	while (1) read_from_client(live_procs, argv[1]);

    unlink (PIPE_NAME);
    
    g_hash_table_destroy(live_procs);

	return 0;
}
