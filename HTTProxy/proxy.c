#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define MAX_CLIENT_NUM 32
#define BUFFER_LEN 4096

int main(int argc, char *argv[])
{
	int listenfd, connfd, i, j;
	struct sockaddr_in serv_addr;
	char recv_buff[BUFFER_LEN];
	char parser[BUFFER_LEN];

	if (argc != 2) {
		printf("Usage: %s <port number>\n", argv[0]);
		return 0;
	}
	printf("Welcome to a HTTP proxy server!\n");

	//Create a listenting socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0x00, sizeof(serv_addr));

	//Setup a listening socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	//Bind and listen
	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return 0;
	}
	if (listen(listenfd, MAX_CLIENT_NUM) < 0) {
		perror("listen");
		return 0;
	}

	//Accept
	if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0) {
		perror("accept");
		return 0;
	}

	//Read the header
	memset(recv_buff, 0x00, BUFFER_LEN);
	memset(parser, 0x00, BUFFER_LEN);
	if (read(connfd, recv_buff, BUFFER_LEN - 1) < 0) {
		perror("read");
		return 0;
	}

	//Parser the header
	i = 0;
	while (recv_buff[i] != '\0') {
		for (j = i; (recv_buff[i] != '\n') && (recv_buff[i] != '\0'); i++)
			parser[i - j] = recv_buff[i];
		parser[i - j] = '\0';
		if (recv_buff[i] == '\n') {
			i++;
			printf("%s\n", parser);
		}
	}

	//TODO: Modify the header

	//Close the socket
	close(connfd);

	return 0;
}
