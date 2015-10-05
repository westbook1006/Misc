#ifndef _PIPE_SHELL_H
#define _PIPE_SHELL_H

#include <stdio.h>
#include <sys/types.h>

//#define DEBUG
int cmd_valid(char *cmd);
int input_valid(char *cmd);

typedef struct _node{
	char *cmd;
	int fd[2];
	pid_t childpid;
	struct _node *next;
} node;

node *cmdhead = NULL;
void add_cmdlist(char *cmd);
void free_cmdlist();
void print_cmdlist();

void cmd_run();
void cmd_wait();

#endif /* _PIPE_SHELL_H */
