#include "common.h"

#define MESSAGE_BUFF 1024

void message_server(int fd, void * info, int len) {
	char *buff[MESSAGE_SIZE];
	memcpy(buff, info, len);
	if (write(fd, buff, MESSAGE_SIZE) == -1) {
		perror("Error on write at start");
	}
}

/**
 * Escrever informação inicial sobre processo, quando é criado por cliente
 * Args: fd do servidor, pid do novo processo, nome do processo
 */
void ping_init (int fd, pid_t pid, char * name) {
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	InfoInit new = {
		.type = START,
		.procInit = {
			.pid = pid,
			.time = current_time
		}
	};

	strcpy(new.procInit.name, name); // mudar para strings de tamanho dinamico no futuro?

	message_server(fd, &new, sizeof(InfoInit));
}

/**
 * Escrever informação final sobre processo, quando é terminado no cliente
 * Args: fd do servidor, pid do processo terminado
 */
void ping_end (int fd, pid_t pid) {
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	InfoEnd new = {
		.type = END,
		.procEnd = {
			.pid = pid,
			.time = current_time
		}
	};

	message_server(fd, &new, sizeof(InfoEnd));
}

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
			ret = pipeline_execute(argv+3);
		}
	}
	return ret;
}
