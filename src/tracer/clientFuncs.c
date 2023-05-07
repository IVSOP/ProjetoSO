#include "clientFuncs.h"

void message_server(int fd, void * info, int len) {
	char *buff[MESSAGE_SIZE]; // isto n devia ser posto a 0s? Senão passava direto o info para o write n?
	memcpy(buff, info, len);
	if (write(fd, buff, MESSAGE_SIZE) == -1) {
		perror("Error on write at start/end");
	}
}

/**
 * Escrever informação inicial sobre processo, quando é criado por cliente
 * Args: fd do servidor, pid do novo processo, nome do processo
 */
void ping_init (int fd, pid_t pid, char * name, struct timeval * time) {

	InfoInit new = {
		.type = START,
		.procInit = {
			.pid = pid,
			.time = *time
		}
	};
	strcpy(new.procInit.name, name); // mudar para strings de tamanho dinamico no futuro?

	printf("name being written is %s\n", new.procInit.name);

	message_server(fd, &new, sizeof(InfoInit));
}

/**
 * Escrever informação final sobre processo, quando é terminado no cliente
 * Args: fd do servidor, pid do processo terminado
 */
void ping_end (int fd, pid_t pid, long int totalTime, cmdType type) {
	
	InfoEnd new = {
		.type = END,
		.procEnd = {
			.pid = pid,
			.time = totalTime,
			.type = type
		}
	};
	message_server(fd, &new, sizeof(InfoEnd));
}
