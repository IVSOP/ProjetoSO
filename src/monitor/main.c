#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "defines.h"

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
int read_ping_init() {
    return 0;
}

int main (int argc, char **argv) {
    // fazer pipe
	if (mkfifo(PIPE_NAME, 0600) != 0) {
		perror("Error making pipe");
	}

    //ler de clientes
    //int fd = open(PIPE_NAME, O_RDONLY);

	return 0;
}
