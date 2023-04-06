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

void send_status_request() {
	pid_t pid = getpid();

	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	if (mkfifo(path, 0600) != 0) {
		perror("Error making pipe");
	}

	InfoStatus info = {
		.type = STATUS,
		.pid = pid
	};

	int fd = open(PIPE_NAME, O_WRONLY);

	if (write(fd, &info, sizeof(InfoStatus)) == -1) {
		perror("Error sending status request");
	}

	close(fd);

	fd = open(path, O_RDONLY);

	char buff[MESSAGE_BUFF]; // malloc?????

	ssize_t bread;
	while ((bread = read(fd, buff, MESSAGE_BUFF)) > 0) {
		if (write(STDOUT_FILENO, buff, bread) == -1) {
			perror("Error writing status to stdout");
		}
	}

	close(fd);
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
	} else if (strcmp(argv[1], "status") == 0) {
		send_status_request();
	}
	return ret;
}
