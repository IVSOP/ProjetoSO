#include "defines.h"

#define INPUT_BUFF 1024

typedef void parse_funcs (char *buff, int *bread);

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

void parse_init(char *buff, int *bread) {
	(*bread) += sizeof(InfoInit);
	InfoInit *res = malloc(sizeof(InfoInit));
	memcpy(res, buff, sizeof(InfoInit));
	// return res;
	printf("Info about start: PID:%d name:%s time: %ld sec %ld usec\n", res->procInit.pid, res->procInit.name, res->procInit.time.tv_sec, res->procInit.time.tv_usec);
}

int parse_inputs(char * buff) {
	int off;
	parse_funcs *funcs[] = { parse_init };
	funcs[(int) buff[0]](buff, &off);
	return off;
}

int read_from_client() {
	char * buff = malloc(sizeof(char) * INPUT_BUFF);
	int fd = open(PIPE_NAME, O_RDONLY);

	int bread;
	int offset = 0;
	while ((bread = read(fd, buff + offset, INPUT_BUFF)) > 0) {
		offset = parse_inputs(buff);
	}


/*
enum msgType {
    START = 0,
    END = 1
};
*/

	return 0;
}

int main (int argc, char **argv) {
    // fazer pipe
	if (mkfifo(PIPE_NAME, 0600) != 0) {
		perror("Error making pipe");
	}

	read_from_client();

    //ler de clientes
    //int fd = open(PIPE_NAME, O_RDONLY);

	return 0;
}
