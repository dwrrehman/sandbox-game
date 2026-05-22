// example udp client.
#include <iso646.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static const char* ip = "10.0.0.193";
static const short port = 25565;

int main() {

	char data[256] = {0};

	int server = socket(AF_INET, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }

	struct sockaddr_in address = {0};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(ip);
	socklen_t size = sizeof address;
	
	strncpy(data, "hello there from space, server!", sizeof data);
	ssize_t error = sendto(server, data, sizeof data, 0, (struct sockaddr*) &address, size);

	printf("connecting to   %s : %hd  ...\n", ip, port);
	memset(data, 0, sizeof data);
	error = recvfrom(server, data, sizeof data, MSG_WAITALL, (struct sockaddr*) &address, &size); 
	printf("response = %s\n", data);

	close(server);
}
