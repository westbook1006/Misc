#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "pipe_shell.h"

int main(int argc, char **argv)
{
	const char *prompt = "mgliu>>";
	const char *delim = "|";
	char *input_cmd, *token, *str;
	int i;

	while (input_cmd = readline(prompt)) {
#ifdef DEBUG
		printf("Input command is: %s\n", input_cmd);
#endif

		if (!input_valid(input_cmd)) {
			printf("syntax error near unexpected token \'|\'\n");
			continue;
		}


		for (i = 1, str = input_cmd; ; i++, str= NULL) {
			token = strtok(str, delim);
			if (token == NULL)
				break;
			if (!cmd_valid(token)) {
				printf("syntax error near unexpected token \'|\'\n");
				break;
			}
#ifdef DEBUG
			printf("\t%d: %s\n", i, token);
#endif
		}

	}

	return 0;
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
