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
#define PIPE_FOLDER "pipes" // ficheiro onde named pipes são guardados
#define PIPE_NAME "pipes/pipe" // pipe principal de clientes para servidor

//macros for executing pipelines
#define PIPELINE_MAX_COMMANDS 16
#define PIPELINE_MAX_PER_COMMAND 16

//macros for exeuting stats commands
#define MAX_STATS_FETCH_PROCS 32 // nº maximo de pids num stats request = MAX_PIDS_FETCHED BY_PROC * x; n sei esse x, supus que era 16
#define MAX_PIDS_FETCHED_BY_PROC 8

#define MESSAGE_BUFF 1024 // qual é a diferença disto para INPUT_BUFF
#define INPUT_BUFF MESSAGE_SIZE * 4
#define MESSAGE_SIZE 256
#define NAME_SIZE 128
#define PATH_SIZE 64

typedef enum {
    START = 0,
    END = 1,
	STATUS = 2,
	STATS_TIME = 3,
	STATS_COMMAND = 4,
	STATS_UNIQ = 5
} msgType;

//struct com informação necessária para o servidor sobre inicio de processo
typedef struct {
	pid_t pid; // na hashtable está struct com pid a mais, já está na key da hashtable, fica para já 
	struct timeval time;
	char name[NAME_SIZE];
} procLogInit;

//struct com informação necessária para o servidor sobre fim de processo
typedef struct {
    pid_t pid;
	long int time;
} procLogEnd;

// struct com procLogInit mais indicação do tipo de mensagem, para o servidor interpretar o tipo de mensagem
typedef struct {
	msgType type;
	procLogInit procInit;
} InfoInit;

// struct com procLogEnd mais indicação do tipo de mensagem, para o servidor interpretar o tipo de mensagem
typedef struct {
	msgType type;
	procLogEnd procEnd;
} InfoEnd;

typedef struct {
	// pid_t pid;
	long int time;
	char name[NAME_SIZE];
} InfoFile;

typedef struct {
	msgType type;
	pid_t pid;
	// void *?????????????
} InfoStatus;

typedef struct {
	msgType type;
	pid_t pid;
	char args[NAME_SIZE];
} InfoStatusArgs;


#endif
