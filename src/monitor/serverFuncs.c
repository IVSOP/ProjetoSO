#include "serverFuncs.h"

typedef void parse_funcs (char *buff, GHashTable * live_procs, char * destFolder);

/** 
 * parse da mensagem de início de um processo
 */
void parse_init(char *buff, GHashTable * live_procs, char * destFolder) { // destFolder não é usado
	//extrair mensagem para struct Init
	buff += offsetof(InfoInit, procInit); // para suportar padding na struct
	procLogInit *res = malloc(sizeof(procLogInit));
	memcpy(res, buff, sizeof(procLogInit));

	// printf("Info about start: PID:%d name:%s time: %ld sec %ld usec\n", res->pid, res->name, res->time.tv_sec, res->time.tv_usec);
	
	//inserção na estrutura
	// printf("Info about start: PID:%d name:%s time: %ld sec %d usec\n", res->pid, res->name, res->time.tv_sec, res->time.tv_usec);
	if (g_hash_table_insert(live_procs, &(res->pid), res) == FALSE) { // se em alguma situação futura se mudar o PID desta struct, a entrada na hashtable quebra!
		perror("Error inserting proc to structure");
		exit(1);
	};
	//free(res) // é freed na destruição da estrutura
}

/** 
 * parse da mensagem de fim de um processo
 */
void parse_end(char *buff, GHashTable * live_procs, char * destFolder) {
	//extrair mensagem para struct End
	buff += offsetof(InfoEnd, procEnd); // para suportar padding na struct
	procLogEnd *res = malloc(sizeof(procLogEnd));
	memcpy(res, buff, sizeof(procLogEnd));

	//pegar no respetivo processo da estrutura
	procLogInit * proc_log = g_hash_table_lookup(live_procs, &(res->pid));

	//meter num ficheiro logs do processo
	InfoFile log;
	log.time = res->time;
	strcpy(log.name, proc_log->name);
	write_to_process_file(res->pid, destFolder, &log);

	//printf("Info about end: PID:%d time: %ld sec %d usec\n", res->pid, res->time.tv_sec, res->time.tv_usec);

	//InfoFile * teste = read_from_process_file(res->pid, destFolder);
	//printf("PID: %d, Info on file: name:%s, time:%ld\n", res->pid, teste->name, teste->time);

	//remover da hashtable a entrada
	g_hash_table_remove(live_procs, &(res->pid));

	free(res); // há necessidade de dar malloc?
}

/** 
 * parse da mensagem de status de um cliente
 */
void parse_status(char *buff, GHashTable * live_procs, char * destFolder) {
	//extrair mensagem para struct End
	buff += offsetof(InfoStatus, pid); // para suportar padding na struct
	
	pid_t pipePid = *(int *)buff;

	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pipePid);
	int pipe_d = open(path, O_WRONLY);

	g_hash_table_foreach (live_procs, printRunningProc, &pipe_d); // mudar depois, forma rafeira de devolver ao cliente o status

	close(pipe_d);
}

void printRunningProc(gpointer key, gpointer value, gpointer pipe_d) {
	procLogInit * procLog = (procLogInit *) value;

	long int resTime;
	struct timeval final_time;
	gettimeofday(&final_time, NULL);
	timersub(&final_time, &(procLog->time), &final_time);
	resTime = final_time.tv_sec * 1000 + final_time.tv_usec / 1000;

	char resultStr[MESSAGE_SIZE]; // mudar valor de buffer depois?? 
	int len = snprintf(resultStr, MESSAGE_SIZE, "%d %s %ld ms\n", (int) procLog->pid, procLog->name, resTime);
	// printf("Output of status: %s\n", resultStr);
	if (write(*(int *)pipe_d, resultStr, len) == -1) {
		perror("Error writing status command");
	}
}

/**
 * Dispatch table que redireciona cada mensagem do cliente para a respetiva função de parse, segundo o primeiro byte lido da mensagem. 0 -> parse_init, 1 -> parse_end
 */
void parse_inputs(char * buff,GHashTable * live_procs, char * destFolder) {
	printf("received %d\n", *(int *)buff);
	parse_funcs *funcs[] = { parse_init, parse_end, parse_status };
	funcs[*(int *)buff](buff, live_procs, destFolder);
}

/**
 * obter a informação de um processo terminando, lendo o seu fichiero
 */
InfoFile * read_from_process_file (pid_t pid, char *folder) {
	InfoFile *info = malloc(sizeof(InfoFile));
	
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	sprintf(end, "/%d", pid); // itoa()???????? // o snprintf acrescenta o número do PID ao path do ficheiro .../PID
	
	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("error opening end proc file");
	}

	if (read(fd, info, sizeof(InfoFile)) == -1) {
		perror("Error opening process file");
	}

	close(fd);
	return info;
}

/**
 * escrever a informação de um processo terminando, criando um ficheiro
 */
void write_to_process_file (pid_t pid, char * folder, InfoFile * info) {
	//criar path final do ficheiro
	char path[PATH_SIZE];
	char *end = stpncpy(path, folder, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	// printf("endPath: %s\n", path);

	//escrever para o ficheiro
	int fd = open(path, O_WRONLY | O_CREAT, 0640); // O_TRUNC é preciso?
	if (fd == -1) {
		perror("Error opening end proc file");
	}

	if (write(fd, info, sizeof(InfoFile)) < (int)sizeof(InfoFile)) {
		perror("Error writing to process file");
	}

	close(fd);
}

/**
 * função que lê do pipe  INPUT_BUFF bytes, consecutivamente, e segue os bytes para parse
 */
int read_from_client(GHashTable * live_procs, char * dest_folder) {
	char * buff = malloc(sizeof(char) * INPUT_BUFF);
	int fd = open(PIPE_NAME, O_RDONLY); // devia ser aqui ou no monitor?

	int bread;
	int offset;
	while ((bread = read(fd, buff, INPUT_BUFF)) > 0) {
		for (offset = 0; offset < bread; offset += MESSAGE_SIZE) {
			parse_inputs(buff + offset, live_procs, dest_folder);
		}
	}

	free(buff);

	return 0;
}

void send_to_client(pid_t pid, void *info, size_t len) {
	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	int fd = open(path, O_RDONLY);

	if (write(fd, info, len) == -1) {
		perror("Error writing to client");
	}

	close(fd);
}
