#ifndef DEFINES_H
#define DEFINES_H

#define PIPE_NAME "pipes/pipe"

enum msgType {
    Start = 0,
    End = 1
};

typedef struct procLog {
	int pid;
	char name[100];
	int time; // mudar tipo depois 
} procLog;

#endif
