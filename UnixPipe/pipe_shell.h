#ifndef _PIPE_SHELL_H
#define _PIPE_SHELL_H

#include <stdio.h>

#define DEBUG
int cmd_valid(char *cmd);
int input_valid(char *cmd);

typedef struct _node{
	char *cmd;
	struct _node *next;
} node;

node *cmdhead = NULL;
void add_cmdlist(char *cmd);
void free_cmdlist();
void print_cmdlist();

#endif /* _PIPE_SHELL_H */
