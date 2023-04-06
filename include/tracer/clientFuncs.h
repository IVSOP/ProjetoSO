#ifndef CLIENTFUNCS_H
#define CLIENTFUNCS_H

#include "common.h"

#define MESSAGE_BUFF 1024

void message_server(int fd, void * info, int len);
void ping_init (int fd, pid_t pid, char * name);
void ping_end (int fd, pid_t pid);

#endif
