#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "amted_server.h"
#include "threadpool.h"

static void free_node(node *nd)
{
	free(nd->path);
	free(nd->read_data);
	free(nd);
}

node *read_list = NULL;
threadpool pool;
static void sig_usr(int signum)
{
	node *tmp = read_list, *tmp1;

	while(tmp) {
		tmp1 = tmp;
		tmp = tmp->next;
		free_node(tmp1);
	}
	destroy_threadpool(pool);
}

static int set_non_blocking(int sock)
{
	int status;
	if((status = fcntl(sock, F_GETFL)) < 0) {
		perror("fcntl get");
		return -1;
	}
	if (fcntl(sock, F_SETFL, status | O_NONBLOCK) < 0) {
		perror("fcntl set");
		return -1;
	}
	
	return 1;
}

int check_exist(char *path) 
{
	int str_len = strlen(path);
	path[str_len - 1] = '\0';

	if (access(path, F_OK))
		return 0;

	return 1;
}

static void read_file(node *nd)
{
	int fd;

	if ((fd = open(nd->path, O_RDONLY)) < 0) {
		perror("open");
		write(nd->pipefd[1], "F", 1);
		return;
	}

	if (read(fd, nd->read_data, MAX_READ) < 0) {
		perror("read");
		write(nd->pipefd[1], "F", 1);
		return;
	}

	close(fd);

	write(nd->pipefd[1], "T", 1);
}

int main(int argc, char **argv)
{
	int listenfd = 0, connfd = 0, epfd, i, nfds, sockfd, n, flag;
	struct sockaddr_in serv_addr;
	char *tmp_path;
	struct epoll_event ev, events[EPOLL_NUM];
	node *tmp, *tmp1;
	struct sigaction sa, sb;

	if (argc != 3) {
		printf("Usage: <%s> <IPv4 address> <port number>\n", argv[0]);
		return 0;
	}

	printf("Welcome to the AMTED server!\n");

	//Register signal processing functions
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
	sb.sa_handler = sig_usr;
	sigaction(SIGINT, &sb, NULL);
	sigaction(SIGTERM, &sb, NULL);

	//Create a listening socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0x00, sizeof(serv_addr));

	//Setup a listening socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);

	//Bind and listen	
	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return 0;
	}
	if (listen(listenfd, MAX_CLIENT_NUM) < 0) {
		perror("listen");
		return 0;
	}
	//Set non-blocking
	if (set_non_blocking(listenfd) < 0)
		return 0;

	//Add to epoll
	if ((epfd = epoll_create(EPOLL_NUM)) < 0) {
		perror("epoll_create");
		return 0;
	}
	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
		perror("epoll_ctl");
		return 0;
	}

	//Create work thread pool
	pool = create_threadpool(MAXT_IN_POOL);

	for (;;) {
		if ((nfds = epoll_wait(epfd, events, EPOLL_NUM, -1)) < 0) {
			perror("epoll_wait");
			return 0;
		}

		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == listenfd) { // New connection
				printf("List to the client\n");
				if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0) {
					perror("accept");
					return 0;
				}
				if (set_non_blocking(connfd) < 0)
					return 0;

				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
					perror("epoll_ctl");
					return 0;
				}
			} else if (events[i].events & EPOLLIN) { // Read
				printf("Get client request\n");
				if ((sockfd = events[i].data.fd) < 0)
					continue;

				flag = 0;
				tmp_path = (char *)malloc(PATH_LENGTH * sizeof(char));
				memset(tmp_path, 0x00, PATH_LENGTH * sizeof(char));
				if ((n = read(sockfd, tmp_path, PATH_LENGTH)) < 0) {
					perror("read");
					close(sockfd);
				}

				tmp = read_list;
				while (tmp) {
					if (tmp->pipefd[0] == events[i].data.fd) {
						if (!strcmp(tmp_path, "T")) {
							if (write(tmp->sockfd, tmp->read_data, strlen(tmp->read_data) + 1) < 0) {
								perror("write");
								return 0;
							} 
						}
						else {
							if (write(tmp->sockfd, "Read Failed\n", 12) < 0) {
								perror("write");
								return 0;
							}
						}
						flag = 1;
						//free resource
						if (tmp == read_list)
							read_list = tmp->next;
						else
							tmp1->next = tmp->next;
						free_node(tmp);
						break;
					}
					tmp1 = tmp;
					tmp = tmp->next;
				}

				if (flag)
					continue;

				if (n == 0) {
					close(sockfd);
					events[i].data.fd = -1;
				} else {
					if (check_exist(tmp_path)) {
						//Create a node
						tmp = (node *)malloc(sizeof(node));
						memset(tmp, 0x00, sizeof(tmp));
						tmp->sockfd = sockfd;
						if (pipe(tmp->pipefd)) {
							perror("pipe");
							return 0;
						}
						tmp->path = tmp_path;
						tmp->read_data = (char *)malloc(MAX_READ);
						memset(tmp->read_data, 0x00, MAX_READ);
						tmp->next = NULL;

						//Add to a list
						if (!read_list)
							read_list = tmp;
						else {
							tmp1 = read_list;
							while(tmp1->next)
								tmp1 = tmp1->next;
							tmp1->next = tmp;
						}

						//Dispatch a thread
						dispatch(pool, read_file, tmp);

						ev.data.fd = tmp->pipefd[0];
						ev.events = EPOLLIN | EPOLLET;
						if (epoll_ctl(epfd, EPOLL_CTL_ADD, tmp->pipefd[0], &ev) < 0) {
							perror("epoll_ctl");
							return 0;
						}

					} else {
						printf("The file %s doesn't exist!\n", tmp_path);
						free(tmp_path);
						continue;
					}
				}
			} else
				;
		}
	}
	return 0;
}
