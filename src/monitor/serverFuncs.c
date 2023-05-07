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
	if (proc_log == NULL) {
		perror("error fetching proc PID");
	}
	//meter num ficheiro logs do processo
	InfoFile log;
	log.time = res->time;
	log.type = res->type;
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

	close(pipe_d); // fechar o pipe de comunicação do status
}

// recebe uma InfoStatusArgs no buff
// a string que esta contém são os PIDs separados por ';'
void parse_stats_time (char *buff, GHashTable * live_procs, char * destFolder) {
	InfoStatusArgs *info = (InfoStatusArgs *)buff;
	
	char path[PATH_SIZE], *end;

	long int resTime = 0, temp;

	char *str = info->args;

	int procLog;

	//parte do path do pipe de resposta de stats, falta só o pid específico no fim do path, que é adicionado nos forks;
	end = stpncpy(path, destFolder, PATH_SIZE - 1);
	end[0] = '/';

	char * pids[MAX_STATS_FETCH_PROCS][MAX_PIDS_FETCHED_BY_PROC+1]; // guarda pids do pedido de stats, no formato de string

	int numberOfFork = splitPIDs(str,pids); // split dos PIDs de input

	int p[2];
	if (pipe(p) == -1) {
		perror("Error opening anonymous pipe");
		exit(1);
	}

	int i;
	for (i = 0; i <= numberOfFork; i++) {
		if (fork() == 0) {
			close(p[0]); // n vai ler do pipe
			for(int k=0; pids[i][k] != NULL; k++) {
				strcpy(end + 1, pids[i][k]);
				printf("filho: %d iter: %d path:%s\n", i, k, path);
				if ((procLog = open(path, O_RDONLY)) == -1) {
					perror("Error opening file on child stats search");
					continue; // saltar para próxima iteração visto que este proc n existe em registo
				}
				if (read(procLog, &temp, sizeof(long int)) == -1) {
					perror("Error reading proc file on child stats search");
				}
				close(procLog);
				resTime += temp;
			}
			if (write(p[1],&resTime,sizeof(long int)) == -1) {
				perror("Error writing totaltime to pipe");
			}
			close(p[1]);
			printf("filho: %d totaltime: %ld\n",i,resTime);
			_exit(-1);
		}
	}
	close(p[1]);
	while(read(p[0],&temp,sizeof(long int)) > 0) {
		resTime += temp;
	}
	close(p[0]);
	int status;
	while (wait(&status) != -1)
		if (!WIFEXITED(status) || WEXITSTATUS(status) < 0)
			perror("Error executing child process");

	//abrir pipe de volta para o cliente com a resposta 
	end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1); // usar só p sprintf e mais nada???
	sprintf(end, "/%d", info->pid);
	int pipe_d = open(path, O_WRONLY);

	int len = snprintf(path, PATH_SIZE, "Total execution time is %ld ms\n", resTime);
	if (write(pipe_d, path, len) == -1) {
		perror("Error sending info back to client");
	}

	close(pipe_d);
}

// recebe uma InfoStatusArgs no buff - contém string de PIDs separados por ';'
void parse_stats_command (char *buff, GHashTable * live_procs, char * destFolder) {
	InfoStatusArgs *info = (InfoStatusArgs *)buff;
	
	char path[PATH_SIZE], *end;

	int count = 0, temp;

	char *str = info->args;

	int procLog;

	end = stpncpy(path, destFolder, PATH_SIZE - 1);
	end[0] = '/';

	char *pids[MAX_STATS_FETCH_PROCS][MAX_PIDS_FETCHED_BY_PROC+1]; // guarda pids do pedido de stats, no formato de string
	
	char *prog = strsep(&str, ";"); // nome do programa a procurar
	printf("%s\n", str);

	int numberOfFork = splitPIDs(str,pids); // split dos PIDs de input

	int p[2];
	if (pipe(p) == -1) {
		perror("Error opening anonymous pipe");
		exit(1);
	}

	int i;
	for (i = 0; i <= numberOfFork; i++) {
		if (fork() == 0) {
			close(p[0]); // n vai ler do pipe
			InfoFile fileLog;
			for(int k=0; pids[i][k] != NULL; k++) {
				strcpy(end + 1, pids[i][k]);
				printf("filho: %d iter: %d path:%s\n", i, k, path);
				if ((procLog = open(path, O_RDONLY)) == -1) {
					perror("Error opening file on child stats search");
					continue; // saltar para próxima iteração visto que este proc n existe em registo
				}
				if (read(procLog, &fileLog, sizeof(InfoFile)) == -1) { // tiro a struct toda porque terá de ler type e args
					perror("Error reading proc file on child stats search");
				}
				if (fileLog.type == SINGLE) { // processo é singular
					if (strcmp(fileLog.name, prog) == 0) count ++;
				}
				else { //processo é pipeline
					char * progs[MAX_PIDS_FETCHED_BY_PROC];
					int nOfProgs = splitProgs(fileLog.name, progs); // split dos progs numa pipeline para string
					for (int j=0; j<nOfProgs;j++) {
						if (strcmp(progs[j], prog) == 0) count ++;
					}
				}
				close(procLog);
			}
			if (write(p[1],&count,sizeof(int)) == -1) {
				perror("Error writing total count to pipe");
			}
			close(p[1]);
			//printf("filho: %d totalcount: %ld\n",i,count);
			_exit(-1);
		}
	}

	close(p[1]);
	while(read(p[0],&temp,sizeof(long int)) > 0) {
		count += temp;
	}
	close(p[0]);

	int status;
	while (wait(&status) != -1)
		if (!WIFEXITED(status) || WEXITSTATUS(status) < 0)
			perror("Error executing child process");

	//abrir pipe de volta para o cliente com a resposta
	end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1); // usar só p sprintf e mais nada???
	sprintf(end, "/%d", info->pid);
	int pipe_d = open(path, O_WRONLY);

	int len = snprintf(path, PATH_SIZE, "%s was executed %d times\n", prog, count);
	if (write(pipe_d, path, len) == -1) {
		perror("Error sending info back to client");
	}

	close(pipe_d);
}

// recebe uma InfoStatusArgs no buff
// a string que esta contém são os PIDs separados por ';'
void parse_stats_uniq (char *buff, GHashTable * live_procs, char * destFolder) {
	InfoStatusArgs *info = (InfoStatusArgs *)buff;
	
	char path[PATH_SIZE], *end;
	char *str = info->args;

	int procLog;

	end = stpncpy(path, destFolder, PATH_SIZE - 1);
	end[0] = '/';

	char *pids[MAX_STATS_FETCH_PROCS][MAX_PIDS_FETCHED_BY_PROC+1]; // guarda pids do pedido de stats, no formato de string
	int numberOfFork = splitPIDs(str,pids);

	int p[2];
	if (pipe(p) == -1) {
		perror("Error opening anonymous pipe");
		exit(1);
	}

	int i;
	for (i = 0; i <= numberOfFork; i++) {
		if (fork() == 0) {
			close(p[0]); // n vai ler do pipe
			GPtrArray * uniqStrings = g_ptr_array_new_with_free_func(free); // meti array dinâmico, porque cada proc podia ter PIPELINE_MAX_COMMANDS o que acaba por ser um array com muito espaço desperdiçado (total: PIPELINE_MAX_COMMANDS * MAX_PIDS FETCHED BY PROC)
			InfoFile fileLog;
			for(int k=0; pids[i][k] != NULL; k++) {
				strcpy(end + 1, pids[i][k]);
				//printf("filho: %d iter: %d path:%s\n", i, k, path);
				if ((procLog = open(path, O_RDONLY)) == -1) {
					perror("Error opening file on child stats search");
					continue; // saltar para próxima iteração visto que este proc n existe em registo
				}
				if (read(procLog, &fileLog, sizeof(InfoFile)) == -1) { // tiro a struct toda porque terá de ler type e args
					perror("Error reading procfile on child stats search");
				}
				if (fileLog.type == SINGLE) { // processo é singular
					addUniqueProg(fileLog.name,uniqStrings);
				}
				else { //processo é pipeline
					char * progs[MAX_PIDS_FETCHED_BY_PROC];
					int nOfProgs = splitProgs(fileLog.name, progs); // split dos progs numa pipeline para string
					for (int j=0; j<nOfProgs;j++)
						addUniqueProg(progs[j],uniqStrings);

				//printf(">string lida ficheiro: %s\n",procName);
				}
				close(procLog);
			}

			int occupied = uniqStrings->len; // contabiliza o nº de strings no array uniqStrings
			for(int j=0;j<occupied; j++) {
				//printf(">>pos %d: string: %s\n",j,uniqStrings[j]); // debug
				if (write(p[1],g_ptr_array_index(uniqStrings,j),NAME_SIZE) == -1) { 
					perror("Error writing progName to pipe");
				}
			}
			close(p[1]);
			g_ptr_array_free(uniqStrings,TRUE);
			_exit(1);
		}
	}

	close(p[1]); // fechar o write do pipe

	GPtrArray * uniqStringsTotal = g_ptr_array_new_with_free_func(free); // array dinâmico para o array final

	char procName[NAME_SIZE]; // buffer para o qual se copia o nome do procs terminados

	//printf("No pai:---\n");
	while(read(p[0],procName,NAME_SIZE) > 0) { // para cada string recebida de filhos
		addUniqueProg(procName,uniqStringsTotal);
	}

	close(p[0]);

	int status;
	while (wait(&status) != -1)
		if (!WIFEXITED(status) || WEXITSTATUS(status) < 0)
			perror("Error executing child process");

	//abrir pipe de volta para o cliente com a resposta
	end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1); // usar só p sprintf e mais nada???
	sprintf(end, "/%d", info->pid);
	int pipe_d = open(path, O_WRONLY);

	int occupied = uniqStringsTotal->len;
	//printf("arrayLen: %d\n", occupied); //debug
	
	char resultStr[MESSAGE_SIZE]; // mudar valor de buffer depois?? 
	int len;
	for(i=0;i<occupied;i++) {
		len = snprintf(resultStr, MESSAGE_SIZE, "%s\n", (char *) g_ptr_array_index(uniqStringsTotal, i));
		if (write(pipe_d,resultStr, sizeof(char)*len) == -1) {
			perror("Error sending info back to client");
		}
	}
	g_ptr_array_free(uniqStringsTotal,TRUE);

	close(pipe_d);
}

//Dada uma string e um GPtrArray de strings, coloca a string no array se ainda n existir
int addUniqueProg(char * progName, GPtrArray * array) {
	int flag=1;
	int occupied = array->len;
	//printf("nome recebido: %s\n",progName);
	for(int j = 0; j < occupied && flag; j++) { // testar se o nome já consta na lista
		if (strcmp(progName, g_ptr_array_index(array,j)) == 0) {
			flag = 0;
			//printf("ja existe na lista %s\n", (char *) g_ptr_array_index(array,j));
		}
	}
	if (flag) g_ptr_array_add(array,strndup(progName, NAME_SIZE)); // se não constar, adicionar ao array (meter por iSort no futuro?)
	
	//for (int j = 0; j < (int) array->len; j++) printf("pos [%d] %s\n", j, (char *) g_ptr_array_index(array,j));
	return 0;
}

//Recebe string de formato "PID1;PID2;...;PIDN" e array de output
//Mete em cada pos do array um pid
//De acordo com max de pids permitidos em macros para cada processo, retorna nº de procs necessários
int splitPIDs(char * input, char * pids[MAX_STATS_FETCH_PROCS][MAX_PIDS_FETCHED_BY_PROC+1]) {
	char * res;
	int numberOfFork = 0,i = 0;
	while(numberOfFork < MAX_STATS_FETCH_PROCS && (res = strsep(&input, ";")) != NULL) { // n era mais fácil passar do cliente só um array de pids no formato string, sem os ';'?
		if (i == MAX_PIDS_FETCHED_BY_PROC) {
			pids[numberOfFork][i] = NULL;
			//printf("%s [%d][%d]\n",pids[numberOfFork][i],numberOfFork,i); 
			numberOfFork++;
			pids[numberOfFork][0] = res;
			//printf("%s [%d][%d]\n",pids[numberOfFork][0],numberOfFork,0); 
			i = 1;
		}
		else {
			pids[numberOfFork][i] = res;
			//printf("%s [%d][%d]\n",pids[numberOfFork][i],numberOfFork,i); 
			i++;
		}
	}
	pids[numberOfFork][i] = NULL;
	//printf("------- %s [%d][%d]\n",pids[numberOfFork][i],numberOfFork,i);
	return numberOfFork;
}

//Recebe pipeline de cmds no formato "NOMECMD1 | NOMECMD2 | ... | NOMECMDN"
// Mete um nomeCMD em cada pos do array progs
//Destrói a string recebida

int splitProgs(char * input, char * progs[PIPELINE_MAX_COMMANDS]) {
	char * res;
	int i = 0;
	//partição dos comandos em cada array
	printf("string orig: %s\n",input);
	while (i < PIPELINE_MAX_COMMANDS && (res = strsep(&input, " ")) != NULL) {
		if (res[0] != '|') {
			progs[i] = res;
			//printf("cmd[%d][%d] = %s (%s)\n", i, j, cmd[i][j], res);
			i++;
		}
	}
	//for(int j=0; j < i ; j++) printf("pos [%d] prog: [%s]\n",j,progs[j]);
	//printf("total progs: %d\n",i);

	return i;
}

void printRunningProc (gpointer key, gpointer value, gpointer pipe_d) {
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
	parse_funcs *funcs[] = { parse_init, parse_end, parse_status, parse_stats_time, parse_stats_command, parse_stats_uniq};
	funcs[*(int *)buff](buff, live_procs, destFolder);
}

/**
 * obter a informação de um processo terminando, lendo o seu ficheiro
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
		perror("Error reading from process file");
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
