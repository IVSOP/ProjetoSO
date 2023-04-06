#ifndef FILEFUNTIONS_H
#define FILEFUNTIONS_H

#include "common.h"

InfoFile * read_from_process_file(pid_t pid, char *folder);
void write_to_process_file (pid_t pid, char * folder, InfoFile * info);
int read_from_client();
void send_to_client(pid_t pid, void *info, size_t len);


#endif