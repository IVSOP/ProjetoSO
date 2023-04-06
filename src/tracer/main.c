#include "clientFuncs.h"

/**
 * Execução de comando individual
 * Args: argumentos recebidos do utilizador
 */
int simple_execute(char **args) {
	int fd = open(PIPE_NAME, O_WRONLY);
	// sem error checking por agora
	pid_t new_pid = getpid();
	if (fork() == 0) {
		ping_init(fd, new_pid, args[0]);
		execvp(args[0], args);
		// _exit(0);
	}
	wait(NULL);
	ping_end(fd, new_pid); 
	close(fd);
	return 0;
}

/**
 * Execução de comando em pipeline
 */
int pipeline_execute(char **args) {
	return 0;
}

int main (int argc, char **argv) {
	int ret = 0;
	if (argc < 2) return 1;

	// maneira rota de usar flags mas não interessa
	if (strcmp(argv[1], "execute") == 0) { // execute individual
		if (strcmp(argv[2], "-u") == 0) {
			ret = simple_execute(argv + 3);
		}
		else if (strcmp(argv[2], "-p") == 0) { // execute em pipeline
			ret = pipeline_execute(argv + 3);
		}
	}
	return ret;
}
