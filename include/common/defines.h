#include <unistd.h>
#include <sys/types.h>
#ifndef DEFINES_H
#define DEFINES_H
#define PIPE_NAME "pipes/pipe"

enum msgType {
    START = 0,
    END = 1
};

typedef struct procLogInit {
	pid_t pid;
	char name[128];
	int time; // mudar tipo depois 
} procLogInit;

typedef struct procLogEnd {
    pid_t pid;
    int time; // mudar tipo depois 
} procLogEnd;

typedef struct infoInit {
	enum msgType type;
	procLogInit procInit;
} InfoInit;

typedef struct infoEnd {
	enum msgType type;
	procLogEnd procEnd;
} InfoEnd;

#endif
