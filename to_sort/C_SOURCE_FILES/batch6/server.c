// example udp server.
#include <iso646.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static const unsigned short port = 12000;

static inline void print_client_address(unsigned char ip[16]) {
	for (int i = 0; i < 16; i += 2) {
		printf("%02hhx%02hhx", ip[i], ip[i + 1]);
		if (i < 14) printf(":");
	}
}

int main() {
	char data[256] = {0};

	int client = socket(PF_INET6, SOCK_DGRAM, 0);
	if (client < 0) { perror("socket"); abort(); }

	setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	setsockopt(client, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));	

	struct sockaddr_in6 server_address = {0}, client_address;
	socklen_t client_length = sizeof client_address;
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;
	
	int result = bind(client, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	printf("listening for client on %hu...\n", port);
	ssize_t error = recvfrom(client, data, sizeof data, MSG_WAITALL, (struct sockaddr*)&client_address, &client_length); 
	printf("%s\n", data);
	
	printf("received from: ");
	print_client_address(client_address.sin6_addr.s6_addr);
	puts("");

	strncpy(data, "thanks for the response.", sizeof data);
	error = sendto(client, data, sizeof data, 0, (struct sockaddr*)&client_address, client_length);

	close(client);
}
