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
#include <stddef.h>
#include <glib.h>

#ifndef DEFINES_H
#define DEFINES_H
#define PIPE_FOLDER "pipes"
#define PIPE_NAME "pipes/pipe"

#define MESSAGE_SIZE 256
#define NAME_SIZE 128

typedef enum {
    START = 0,
    END = 1,
	STATUS = 2,
} msgType;

typedef struct {
	pid_t pid; // na hashtable está struct com pid a mais, já está na key da hashtable, fica para já 
	struct timeval time;
	char name[NAME_SIZE];
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

typedef struct {
	// pid_t pid;
	long int time;
	char name[NAME_SIZE];
} InfoFile;

#endif
