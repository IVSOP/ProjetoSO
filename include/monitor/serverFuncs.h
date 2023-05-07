#ifndef SERVERFUNCS_H
#define SERVERFUNCS_H

#define PATH_SIZE 64
#define INPUT_BUFF MESSAGE_SIZE * 4

#include "common.h"

int splitPIDs(char * input, char * pids[MAX_STATS_FETCH_PROCS][MAX_PIDS_FETCHED_BY_PROC+1]);
void parse_inputs(char * buff,GHashTable * live_procs, char * destFolder); // mudar estes argumentos todos para uma struct (expansível sem requerer restruturações)? 
InfoFile * read_from_process_file(pid_t pid, char * destFolder);
void write_to_process_file (pid_t pid, char * folder, InfoFile * info);
int read_from_client(GHashTable * live_procs, char * dest_folder);
void send_to_client(pid_t pid, void *info, size_t len);
void printRunningProc(gpointer key, gpointer value, gpointer data);

#endif
