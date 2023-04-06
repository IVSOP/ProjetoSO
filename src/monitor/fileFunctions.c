#include "fileFunctions.h"

#define PATH_SIZE 64
#define INPUT_BUFF MESSAGE_SIZE * 4
#define INPUT_BUFF MESSAGE_SIZE * 4

typedef void parse_funcs (char *buff);


void parse_init(char *buff) {
	buff += offsetof(InfoInit, procInit);
	procLogInit *res = malloc(sizeof(procLogInit));
	memcpy(res, buff, sizeof(procLogInit));
	// return res;
	printf("Info about start: PID:%d name:%s time: %ld sec %ld usec\n", res->pid, res->name, res->time.tv_sec, res->time.tv_usec);
}

void parse_end(char *buff) {
	buff += offsetof(InfoEnd, procEnd);
	procLogEnd *res = malloc(sizeof(procLogEnd));
	memcpy(res, buff, sizeof(procLogEnd));
	// return res;
	printf("Info about end: PID:%d time: %ld sec %ld usec\n", res->pid, res->time.tv_sec, res->time.tv_usec);
}

void parse_inputs(char * buff) {
	parse_funcs *funcs[] = { parse_init, parse_end };
	funcs[(int) buff[0]](buff);
}

InfoFile * read_from_process_file(pid_t pid, char *folder) {
	InfoFile *info = malloc(sizeof(InfoFile));
	
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	snprintf(end, end - path, "/%d", pid); // itoa()????????
	
	int fd = open(path, O_RDONLY);
	
	if (read(fd, info, sizeof(InfoFile)) == -1) {
		perror("Error opening process file");
	}

	close(fd);
	return info;
}

void write_to_process_file (pid_t pid, char * folder, InfoFile * info) {
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	snprintf(end, end - path, "/%d", pid);

	int fd = open(path, O_WRONLY | O_CREAT, 0640);

	if (write(fd, info, sizeof(InfoFile)) < (int)sizeof(InfoFile)) {
		perror("Error writing to process file");
	}

	close(fd);
}

int read_from_client() {
	char * buff = malloc(sizeof(char) * INPUT_BUFF);
	int fd = open(PIPE_NAME, O_RDONLY);

	int bread;
	int offset;
	while ((bread = read(fd, buff, INPUT_BUFF)) > 0) {
		for (offset = 0; offset < bread; offset += MESSAGE_SIZE) {
			parse_inputs(buff + offset);
		}
	}

	free(buff);

	return 0;
}

void send_to_client(pid_t pid, void *info, size_t len) {
	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	snprintf(end, end - path, "/%d", pid);

	int fd = open(path, O_RDONLY);

	if (write(fd, info, len) == -1) {
		perror("Error writing to client");
	}

	close(fd);
}
