#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
 
int main(void) {

	int connection = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (connection < 0) { perror("socket()"); exit(1); }
 
	struct sockaddr_in6 server_addr = {0};
	server_addr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, "::1", &server_addr.sin6_addr);
	server_addr.sin6_port = htons(12000);
 
	int r = connect(connection, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (r < 0) { perror("connect()"); exit(1); }
 
	char c = 'a';
	ssize_t n = write(connection, &c, 1);
	if (n < 0) { perror("write"); return 1; }

	n = read(connection, &c, 1);
	if (n < 0) { perror("read()"); exit(1); } 
 
	printf("Received %c from server\n", c);
	close(connection);
}
