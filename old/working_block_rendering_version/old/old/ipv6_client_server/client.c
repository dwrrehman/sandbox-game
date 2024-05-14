// example udp client.
#include <iso646.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static const char* ip = "2601:1c2:4001:ecf0:55a9:3314:77c8:7ceb";
static const short port = 12000;

int main() {
	
	char data[256] = {0};

	int server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }

	struct sockaddr_in6 address = {0};
	address.sin6_family = PF_INET6;
	address.sin6_port = htons(port);
	inet_pton(PF_INET6, ip, &address.sin6_addr); 
	socklen_t size = sizeof address;
	
	strncpy(data, "hello there from space, server!", sizeof data);
	ssize_t error = sendto(server, data, sizeof data, 0, (struct sockaddr*) &address, size);

	printf("connecting to   %s : %hd  ...\n", ip, port);
	memset(data, 0, sizeof data);
	error = recvfrom(server, data, sizeof data, MSG_WAITALL, (struct sockaddr*) &address, &size); 
	printf("response = %s\n", data);

	close(server);
}
