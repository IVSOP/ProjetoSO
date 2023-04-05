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

// abrir e fechar aqui????
void message_server(char * message, size_t len) {
	int fd = open(PIPE_NAME, O_WRONLY); // tirar daqui depois, ja aberto antes?
	// fazer write em loop???????????????????
	if (write(fd, message, len) == -1) {
		perror("Error on write");
	}

	close(fd);
}

void ping_init (int server_d, pid_t pid, char * name) {
	Info new;
	new.type = START;
	new.proc.pid = pid;
	strcpy(new.proc.name,name);
	struct timeval current_time;
	new.proc.time = gettimeofday(&current_time, NULL);
        // printf("seconds : %ld\nmicro seconds : %ld",
        // current_time.tv_sec, current_time.tv_usec);
        // milis: current_time.tv_usec / 1000

	// char * message = malloc(sizeof(char) * MESSAGE_BUFF);
	write(server_d,&new,sizeof(Info));
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
