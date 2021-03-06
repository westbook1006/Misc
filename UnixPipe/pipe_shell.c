#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>

#include "pipe_shell.h"

int main(int argc, char **argv)
{
	const char *prompt = "mgliu>>";
	const char *delim = "| ";
	char *input_cmd, *token, *str;
	int i, invalid;

	while (input_cmd = readline(prompt)) {
#ifdef DEBUG
		printf("Input command is: %s\n", input_cmd);
#endif

		if (!input_valid(input_cmd)) {
			printf("syntax error near unexpected token \'|\'\n");
			continue;
		}

		for (invalid = 0, i = 1, str = input_cmd; ; i++, str= NULL) {
			token = strtok(str, delim);
			if (token == NULL)
				break;
			if (!cmd_valid(token)) {
				printf("syntax error near unexpected token \'|\'\n");
				invalid = 1;
				break;
			}
			add_cmdlist(token);
#ifdef DEBUG
			printf("\t%d: %s\n", i, token);
#endif
		}
#ifdef DEBUG
		if (!invalid) {
			print_cmdlist();
		}
#endif
		if (!invalid) {
			cmd_run();
			cmd_wait();
		}
#ifdef DEBUG
		if (!invalid) {
			print_cmdlist();
		}
#endif
		free_cmdlist();

		usleep(5000);
	}

	return 0;
}

void cmd_wait()
{
	node *tmp = cmdhead;
	int status;

	do {
		if (waitpid(-1, &status, 0) < 0)
			;//perror("waitpid");
	}while(!WIFEXITED(status));

	/*while(tmp != NULL) {
		close(tmp->fd[0]);
		close(tmp->fd[1]);
		tmp = tmp->next;
	}*/

}

void cmd_run()
{
	node *tmp = cmdhead, *tmp1 = cmdhead;
	node *tmp_prev = NULL;
	char *argv[] = {"", NULL};
	pid_t cpid, w;
	int status;

	while (tmp != NULL) {
		cpid = fork();

		if (cpid == -1) {
			perror("fork");
			return;
		}

		if (cpid == 0) {	/* Child process*/
			if (tmp == cmdhead) {
				if (tmp->next != NULL) {
					dup2(tmp->fd[1], 1);
				}
				while(tmp1 != NULL) {
					close(tmp1->fd[0]);
					close(tmp1->fd[1]);
					tmp1 = tmp1->next;
				}
			} else {
				dup2(tmp_prev->fd[0], 0);

				if (tmp->next != NULL)
					dup2(tmp->fd[1], 1);

				while(tmp1 != NULL) {
					close(tmp1->fd[0]);
					close(tmp1->fd[1]);
					tmp1 = tmp1->next;
				}
			}

			if (execvp(tmp->cmd, argv) < 0) {
				perror(tmp->cmd);
			} 
		
			_exit(0);
		} else {
			tmp->childpid = cpid;
		}

		tmp_prev = tmp;
		tmp = tmp->next;
	}

}

void add_cmdlist(char *cmd) 
{
	node *tmp, *tmp1;

	tmp = (node *) malloc(sizeof(node));
	memset(tmp, 0x00, sizeof(node));

	tmp->cmd = cmd;
	pipe(tmp->fd);
	tmp->next = NULL;

	if (cmdhead == NULL) {
		cmdhead = tmp;
	} else {
		tmp1 = cmdhead;
		while(tmp1->next != NULL)
			tmp1 = tmp1->next;
		tmp1->next = tmp;
	}
}

void free_cmdlist()
{
	node *tmp = cmdhead;

	while (cmdhead != NULL) {
		tmp = tmp->next;
		close(cmdhead->fd[0]);
		close(cmdhead->fd[1]);
		free(cmdhead);
		cmdhead =  tmp;
	}
}

void print_cmdlist()
{
	node *tmp = cmdhead;

	while (tmp != NULL) {
		printf("LIST --> %s\n", tmp->cmd);
		tmp = tmp->next;
	}
}

int input_valid(char *cmd)
{
	if (strlen(cmd) < 1)
		return 1;
	else {
		if (cmd[strlen(cmd) - 1] == '|')
			return 0;
	}
}

int cmd_valid(char *cmd)
{
	while(*cmd != '\0') {
		if (!isspace(*cmd)) {
			return 1;
		}
		cmd++;
	}

	return 0;
}
