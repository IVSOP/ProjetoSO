#include "defines.h"

#define INPUT_BUFF MESSAGE_SIZE * 4

#define PATH_SIZE 64

typedef void parse_funcs (char *buff);

void daemonize() {
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

InfoFile * readFromFile(pid_t pid, char *folder) {
	InfoFile *info = malloc(sizeof(InfoFile));
	
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	snprintf(end, end - path, "%d", pid); // itoa()????????
	
	int fd = open(path, O_RDONLY);
	
	if (read(fd, info, sizeof(InfoFile)) == -1) {
		perror("Error opening process file");
	}

	return info;
}

void writeToFile (pid_t pid, char * folder, InfoFile * info) {
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	snprintf(end, end - path, "%d", pid);

	int fd = open(path, O_WRONLY | O_CREAT, 0640);

	if (write(fd, info, sizeof(InfoFile)) < (int)sizeof(InfoFile)) {
		perror("Error writing to process file");
	}
}

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

	return 0;
}

int main (int argc, char **argv) {
    // fazer pipe
	if (mkfifo(PIPE_NAME, 0600) != 0) {
		perror("Error making pipe");
	}

	while(1) read_from_client();

	return 0;
}
