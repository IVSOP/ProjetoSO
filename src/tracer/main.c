#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "defines.h"
#include <fcntl.h>
#include <sys/time.h>
#define MESSAGE_BUFF 1024

typedef struct Info {
	enum msgType type;
	procLog proc;
} Info;

void ping_init (int fd, pid_t pid, char * name) {
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	Info new = {
		.type = START,
		.proc = {
			.pid = pid,
			.time = current_time.tv_usec / 1000
		}
	};
	strcpy(new.proc.name, name);

	// char * message = malloc(sizeof(char) * MESSAGE_BUFF);
	if (write(fd, &new, sizeof(Info)) == -1) {
		perror("Error on write");
	}
}

void simple_execute(char **args) {
	int fd = open(PIPE_NAME, O_WRONLY);
	// sem error checking por agora
	if (fork() == 0) {
		ping_init(fd,getpid(),args[0]);
		execvp(args[0], args);
	}
	close(fd);
}

int main (int argc, char **argv) {
	if (argc < 2) return 1;

	// maneira rota de usar flags mas não interessa
	if (strcmp(argv[1], "execute") == 0) {
		if (strcmp(argv[2], "-u") == 0) {
			simple_execute(argv + 3);
		}
	}
	return 0;
}
