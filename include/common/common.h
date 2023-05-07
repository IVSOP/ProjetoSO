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
#define MAX_STATS_FETCH_PROCS 32 // máx de PIDS fornecidos num pedido de stats
#define MAX_PIDS_FETCHED_BY_PROC 8 // nº de PIDs que cada processo 

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

typedef enum {
	SINGLE = 0,
	PIPELINE = 1
} cmdType;

//struct com informação necessária para o servidor sobre inicio de processo
//procLogInit não contém informação sobre cmd ser singular ou pipe, porque nenhuma funcionalidade durante execução requer essa distinção
typedef struct {
	pid_t pid; // na hashtable está struct com pid a mais, já está na key da hashtable, fica para já 
	struct timeval time;
	char name[NAME_SIZE];
} procLogInit;

//struct com informação necessária para o servidor sobre fim de processo
//procLogEnd contém informação sobre cmd ser singular ou pipe, porque funcionalidades de processos terminados requerem essa distinção
typedef struct {
    pid_t pid;
	long int time;
	cmdType type; 
} procLogEnd;

// struct com procLogInit mais indicação do tipo de mensagem, para o servidor interpretar mensagem inicial
typedef struct {
	msgType type;
	procLogInit procInit;
} InfoInit;

// struct com procLogEnd mais indicação do tipo de mensagem, para o servidor interpretar mensagem final
typedef struct {
	msgType type;
	procLogEnd procEnd;
} InfoEnd;

// structs guardadas em ficheiros de procs terminados
// requer indicação do tipo de comando (singular ou pipe), funcionalidades dependem disso
typedef struct {
	// pid_t pid;
	long int time;
	char name[NAME_SIZE];
	cmdType type;
} InfoFile;

// struct com pedido de status ao servidor, mais indicação do tipo de mensagem
typedef struct {
	msgType type;
	pid_t pid;
	// void *?????????????
} InfoStatus;

// struct com pedido de stats para PIDS dados, ao servidor, mais indicação do tipo de mensagem
typedef struct {
	msgType type;
	pid_t pid;
	char args[NAME_SIZE];
} InfoStatusArgs;


#endif
