#include "clientFuncs.h"

/**
 * Execução de comando individual
 * Args: Comando de input do utilizador
 */
int simple_execute(char * command) {
	int fd = open(PIPE_NAME, O_WRONLY);
	if (fd == -1) {
		perror("Error opening pipe to server");
		exit(fd);
	}

	pid_t new_pid = getpid();

	//split do comando em partes
	char * splitInput[20]; // max param de um comando é quanto?
	char * rest = command;
	int i;
	for (i=0; (splitInput[i] = strtok_r(rest, " ", &rest)) != NULL; i++);

	// feedback ao user
	char userFeedback[NAME_SIZE];

	// ###
	// concatenar os args com o nome do programa 
	// só se pede explicitamente o nome do proc, tho
	// reutiliza a string userFeedback, n está relacionado
	// int j, offset = 0;
	// char *end;
	// for (j = 0; j < i; i++) {
	// 	end = stpcpy(userFeedback + offset, args[i]);
	// 	end[0] = ' ';
	// 	offset = end - userFeedback + 1;
	// }
	// offset--;
	// userFeedback[offset] = '\0';
	// ####

	// tempo inicial
	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	
	//ping inicial ao servidor
	ping_init(fd, new_pid, splitInput[0], &start_time);

	//notificar o utilizador
	// não se pode só usar printf???
	int strSize = snprintf(userFeedback, NAME_SIZE, "Running PID %d\n", new_pid);
	if (write(STDOUT_FILENO, userFeedback, strSize) == -1) {
		perror("Error sending execute info");
	}

	// execução
	if (fork() == 0) {
		execvp(splitInput[0], splitInput);
		_exit(0);
	}

	//esperar pelo resultado
	int status;
	wait(&status);
	if (!WIFEXITED(status) || WEXITSTATUS(status) < 0) {
		perror("Error executing proc on client");
	}

	// tempo final e intervalo total
	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &end_time);
	long int totalTime = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

	// ping final ao servidor
	ping_end(fd, new_pid, totalTime, SINGLE); 

	// notificar o utilizador
	memset(userFeedback, 0, sizeof(char) * NAME_SIZE);
	strSize = snprintf(userFeedback, PATH_SIZE, "Ended in %ld ms\n", totalTime);
	if (write(STDOUT_FILENO, userFeedback, strSize) == -1) {
		perror("Error sending execute info");
	}

	close(fd);
	return 0;
}

/**
 * simple status requests, without args 
 * aka status
 */
void send_status_request() {

	pid_t pid = getpid();

	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	if (mkfifo(path, 0600) != 0) {
		perror("Error making pipe");
	}

	InfoStatus info = {
		.type = STATUS,
		.pid = pid
	};

	int fd = open(PIPE_NAME, O_WRONLY); // abrir pipe para o servidor
	if (fd == -1) {
		perror("Error opening pipe to write");
	}

	if (write(fd, &info, sizeof(InfoStatus)) == -1) {
		perror("Error sending status request");
	}

	close(fd);
	if (fd == -1) {
		perror("Error closing pipe");
	}

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("Error openining pipe to read");
	}

	char buff[MESSAGE_BUFF]; // malloc?????

	ssize_t bread;
	while ((bread = read(fd, buff, MESSAGE_BUFF)) > 0) {
		if (write(STDOUT_FILENO, buff, bread) == -1) {
			perror("Error writing status to stdout");
		}
	}

	close(fd);
	if (fd == -1) {
		perror("Error closing pipe");
	}

	//apaga
	unlink(path);
}

/**
 * status requests with specific args
 * aka stats-time, stats-command, stats-uniq
 */
void send_stats_request_args(msgType type, char ** args) {
	pid_t pid = getpid();

	char path[PATH_SIZE];
	char *end = stpncpy(path, PIPE_FOLDER, PATH_SIZE - 1);
	sprintf(end, "/%d", pid);

	if (mkfifo(path, 0600) != 0) {
		perror("Error making pipe");
	}

	InfoStatusArgs info = {
		.type = type,
		.pid = pid
	};
	
	int i; int offset = 0;

	for (i = 0; args[i] != NULL; i++) { // receber argc?????
		end = stpcpy(info.args + offset, args[i]);
		end[0] = ';';
		offset = end - info.args + 1; // + 1 para manter o ';'
	}
	end[0] = '\0';

	//args terá só PIDS para stats_time e nomeProc+args para stats_command e stats_uniq

	int fd = open(PIPE_NAME, O_WRONLY);

	if (write(fd, &info, sizeof(InfoStatusArgs)) == -1) {
		perror("Error sending status request");
	}

	close(fd);

	fd = open(path, O_RDONLY);

	char buff[MESSAGE_BUFF]; // malloc?????

	ssize_t bread;
	while ((bread = read(fd, buff, MESSAGE_BUFF)) > 0) {
		if (write(STDOUT_FILENO, buff, bread) == -1) {
			perror("Error writing status to stdout");
		}
	}

	close(fd);
	unlink(path);
}

/**
 * executa uma pipeline de comandos
 */
void pipeCommands(char *** cmd, int size) {
	if (size == 1) {
		perror("Command isn´t pipeline");
		exit(2);
	}
	// não é preciso?? ao ter size == 2, nao entra no loop intermedio acho e dá direito
	// if (size == 2) {
	// 	int p[2];
	// 	if (pipe(p) == -1) {
	// 		perror("Error making pipe");
	// 	}

	// 	if (fork() == 0) {
	// 		// filho vai ler
	// 		close(p[1]);
	// 		dup2(p[0], STDIN_FILENO);
	// 		close(p[0]);

	// 		execvp(cmd[1][0], cmd[1]);
	// 		_exit(1); // caso dê erro
	// 	} else {
	// 		// pai vai escrever
	// 		if (fork() == 0) {
	// 			close(p[0]);
	// 			dup2(p[1], STDOUT_FILENO);
	// 			close(p[1]);

	// 			execvp(cmd[0][0], cmd[0]);
	// 			_exit(1);
	// 		}
	// 	}

	// 	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// 	close(p[1]);
	// 	close(p[0]);


	// 	while (wait(NULL) != -1);

	// } else {
		int fd_arr[size - 1][2], i;
		if (pipe(fd_arr[0]) == -1) {
			perror("Error making pipe");
			exit(1);
		}

		if (fork() == 0) {
			dup2(fd_arr[0][1], STDOUT_FILENO);
			close(fd_arr[0][1]);

			close(fd_arr[0][0]); // não vai ler do pipe

			execvp(cmd[0][0], cmd[0]);
			_exit(1);
		}


		for (i = 1; i < size - 1; i++) {
			close(fd_arr[i - 1][1]);
			if (pipe(fd_arr[i]) == -1) {
				perror("Error making pipe");
				exit(1);
			}
			if (fork() == 0) {
				dup2(fd_arr[i - 1][0], STDIN_FILENO);
				close(fd_arr[i - 1][0]);

				dup2(fd_arr[i][1], STDOUT_FILENO);
				close(fd_arr[i][1]);

				execvp(cmd[i][0], cmd[i]);
				_exit(1);
			}
			close(fd_arr[i - 1][0]);
		}

		close(fd_arr[i - 1][1]); // nunca vai escrever para o pipe

		if (fork() == 0) {
			dup2(fd_arr[i - 1][0], STDIN_FILENO);
			close(fd_arr[i - 1][0]);


			execvp(cmd[size - 1][0], cmd[size - 1]);
			_exit(1);
		}
		close(fd_arr[i - 1][0]);
	//}

	int status;
	while (wait(&status) != -1) {
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {
			printf("Child exited with %d\n", WEXITSTATUS(status));
			exit(1);
		}
	}
}

/**
 * processa um comando no formato de pipe
 * parte o comando em substrings de comandos mais pequenos, executadas em pipeline na função pipeCommands()
 */
int pipeline_execute(char * command) {

	//array de comandos, sendo cada comando um array de argumentos
	// char *cmd[PIPELINE_MAX_COMMANDS][PIPELINE_MAX_PER_COMMAND];
	char ***cmd = malloc(PIPELINE_MAX_COMMANDS * sizeof(char **));
	int i, j;

	for (i = 0; i < PIPELINE_MAX_COMMANDS; i++) {
		cmd[i] = malloc(PIPELINE_MAX_PER_COMMAND * sizeof(char *));
	}


	char * res;
	//partição dos comandos em cada array
	for (i = 0, j = 0; i < PIPELINE_MAX_COMMANDS - 1 && j < PIPELINE_MAX_PER_COMMAND && (res = strsep(&command, " ")) != NULL;) {
		if (res[0] == '|') {
			cmd[i][j] = NULL;
			//printf("cmd[%d][%d] = NULL\n", i, j);
			j = 0;
			i++;
		} else {
			cmd[i][j] = res;
			//printf("cmd[%d][%d] = %s (%s)\n", i, j, cmd[i][j], res);
			j++;
		}
	}

	cmd[i][j] = NULL; // necessário para o último comando
	//printf("cmd[%d][%d] = NULL\n---------------------\n", i, j);
	i++;

	// abertura do pipe
	int fd = open(PIPE_NAME, O_WRONLY);
	if (fd == -1) {
		perror("Error opening pipe to server");
		exit(fd);
	}

	char pipeName[NAME_SIZE];
	pipeName[0] = '\0';
	int offset;
	for (offset = 0, j = 0; j < i && offset < NAME_SIZE + 3; j++) { // o +3 vem de cortar no final a secção " | " com '\0'
		offset += sprintf(pipeName + offset,"%s | ",cmd[j][0]);
	}
	pipeName[offset-3] = '\0';
	printf("[%s]\n",pipeName);

	//pipeline vai ficar associada a pid do processo do cliente
	pid_t new_pid = getpid();

	//tempo inicial
	struct timeval start_time;
	gettimeofday(&start_time, NULL);

	//ping inicial ao servidor
	ping_init(fd, new_pid, pipeName, &start_time);

	//notificar o utilizador
	// não se pode só usar printf???
	char userFeedback[NAME_SIZE];
	int strSize = snprintf(userFeedback, NAME_SIZE, "Running PID %d\n", new_pid);
	if (write(STDOUT_FILENO, userFeedback, strSize) == -1) {
		perror("Error sending execute info");
	}
	//execução da pipeline, já faz wait
	pipeCommands(cmd, i);

	// tempo final e intervalo total
	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &end_time);
	long int totalTime = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

	// ping final ao servidor
	ping_end(fd, new_pid, totalTime, PIPELINE); 

	// notificar o utilizador
	memset(userFeedback, 0, sizeof(char) * NAME_SIZE);
	strSize = snprintf(userFeedback, PATH_SIZE, "Ended in %ld ms\n", totalTime);
	if (write(STDOUT_FILENO, userFeedback, strSize) == -1) {
		perror("Error sending execute info");
	}


	//isto já não requer free acho? visto que vem da argv[3]
	for (i = 0; i < PIPELINE_MAX_COMMANDS; i++) {
		free(cmd[i]);
	}

	//free do array inicial
	free(cmd);

	return 0;
}


int main (int argc, char **argv) {
	int ret = 0;
	if (argc < 2) return 1;

	if (strcmp(argv[1], "execute") == 0) { // execute individual
		if (strcmp(argv[2], "-u") == 0) {
			ret = simple_execute(argv[3]);
		}
		else if (strcmp(argv[2], "-p") == 0) { // execute em pipeline
			ret = pipeline_execute(argv[3]);
		}
	} else if (strcmp(argv[1], "status") == 0) {
		send_status_request();
	} else if (strcmp(argv[1], "stats-time") == 0) {
		send_stats_request_args(STATS_TIME, argv + 2);
	} else if (strcmp(argv[1], "stats-command") == 0) {
		send_stats_request_args(STATS_COMMAND, argv + 2);
	} else if (strcmp(argv[1],"stats-uniq") == 0) {
		send_stats_request_args(STATS_UNIQ, argv + 2);
	} else {
		perror("Command invalid");
		exit(1);
	}
	return ret;
}
