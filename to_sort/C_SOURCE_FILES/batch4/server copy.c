#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
 
int main(void) {

	int server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (server < 0) { perror("socket()"); exit(1); }
 
	int reuse_address_value = 1;
	int ret = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse_address_value, sizeof(reuse_address_value));
	if (ret < 0) { perror("setsockopt()"); exit(1); }

	struct sockaddr_in6 server_addr;
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(12000);
 
	ret = bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0) { perror("bind()"); exit(1); }
 
	ret = listen(server, 5);
	if (ret < 0) { perror("listen()"); exit(1); }
 
	struct sockaddr_in6 client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	while (1) {
		printf("listening for incomming client connections on port %u...\n", 12000);

		int connection = accept(server, (struct sockaddr*)&client_addr, &client_addr_len);
		if (connection < 0) { perror("accept()"); exit(1); }
 
		char str_addr[INET6_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET6, &(client_addr.sin6_addr), str_addr, sizeof(str_addr));
		printf("New connection from: %s:%d ...\n", str_addr, ntohs(client_addr.sin6_port));
 
		char ch = 0;
		ssize_t n = read(connection, &ch, 1);
		if (n < 0) { perror("read()"); exit(1); }

		ch++;

		n = write(connection, &ch, 1);
		if (n < 0) { perror("write()"); exit(1); } 

		close(connection);
		printf("Connection closed\n");
	}
}
