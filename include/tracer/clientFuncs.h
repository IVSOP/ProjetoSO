#ifndef CLIENTFUNCS_H
#define CLIENTFUNCS_H

#include "common.h"

void message_server(int fd, void * info, int len);
void ping_init (int fd, pid_t pid, char * name, struct timeval * time);
void ping_end (int fd, pid_t pid, long int totalTime);

#endif
