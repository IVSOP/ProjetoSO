#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>

#ifndef DEFINES_H
#define DEFINES_H
#define PIPE_NAME "pipes/pipe"

typedef enum {
    START = 0,
    END = 1
} msgType;

typedef struct {
	pid_t pid;
	struct timeval time;
	char name[128];
} procLogInit;

typedef struct {
    pid_t pid;
	struct timeval time;
} procLogEnd;

typedef struct {
	msgType type;
	procLogInit procInit;
} InfoInit;

typedef struct {
	msgType type;
	procLogEnd procEnd;
} InfoEnd;

#endif
