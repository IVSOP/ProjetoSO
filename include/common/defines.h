#include <unistd.h>
#include <sys/types.h>
#ifndef DEFINES_H
#define DEFINES_H
#define PIPE_NAME "pipes/pipe"

enum msgType {
    START = 0,
    END = 1
};

typedef struct procLog {
	pid_t pid;
	char name[128];
	int time; // mudar tipo depois 
} procLog;

typedef struct {
	enum msgType type;
	procLog proc;
} StartInfo;

#endif
