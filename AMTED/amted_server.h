#ifndef _AMTED_SERVER_H
#define _AMTED_SERVER_H

#define PATH_LENGTH 256
#define MAX_CLIENT_NUM 16
#define EPOLL_NUM 32
#define MAX_READ 1024*1024*2

typedef struct _node{
	int sockfd;
	int pipefd[2];
	char *path;
	char *read_data;	
	struct _node *next;
}node;

#endif
