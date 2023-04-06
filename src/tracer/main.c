#include "clientFuncs.h"

/**
 * Execução de comando individual
 * Args: argumentos recebidos do utilizador
 */
int simple_execute(char **args) {
	int fd = open(PIPE_NAME, O_WRONLY);
	// sem error checking por agora
	pid_t new_pid = getpid();

	//tempo inicial
	struct timeval start_time;
	gettimeofday(&start_time, NULL);

	//feedback ao user
	char userFeedback[PATH_SIZE];
	int strSize = snprintf(userFeedback, PATH_SIZE, "Running PID %d\n", new_pid);
	write(STDOUT_FILENO, userFeedback, strSize);

	//execução e ping inicial
	if (fork() == 0) {
		ping_init(fd, new_pid, args[0], &start_time);
		execvp(args[0], args);
		_exit(0);
	}
	wait(NULL);

	//tempo total
	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &end_time);
	long int totalTime = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

	//ping final
	ping_end(fd, new_pid, totalTime); 

	//feedback
	memset(userFeedback, 0, sizeof(char)*PATH_SIZE);
	strSize = snprintf(userFeedback, PATH_SIZE, "Ended in %ld ms\n", totalTime);
	write(STDOUT_FILENO, userFeedback, strSize);

	close(fd);
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

void send_stats_time_request(char ** args) {
	pid_t pid = getpid();

	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	if (mkfifo(path, 0600) != 0) {
		perror("Error making pipe");
	}

	InfoStatusArgs info = {
		.type = STATS_TIME,
		.pid = pid
	};
	
	int i; int offset = 0;

	for (i = 0; args[i] != NULL; i++) { // receber argc?????
		end = stpcpy(info.args + offset, args[i]);
		end[0] = ';';
		offset = end - info.args + 1; // + 1 para manter o ';'
	}
	end[0] = '\0';

	int fd = open(PIPE_NAME, O_WRONLY);

	if (write(fd, &info, sizeof(InfoStatusArgs)) == -1) {
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

int pipeline_execute(char ** args) {
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
	} else if (strcmp(argv[1], "status") == 0) {
		send_status_request();
	} else if (strcmp(argv[1], "stats-time") == 0) {
		send_stats_time_request(argv + 2);
	}
	return ret;
}
