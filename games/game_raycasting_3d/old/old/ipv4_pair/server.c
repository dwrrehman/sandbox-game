// example udp server.
#include <iso646.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

static const unsigned short port = 25565;

int main() {

	char data[256] = {0};

	int client = socket(AF_INET, SOCK_DGRAM, 0);
	if (client < 0) { perror("socket"); abort(); }

	setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	setsockopt(client, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));	

	struct sockaddr_in server_address = {0}, client_address;
	socklen_t client_length = sizeof client_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int result = bind(client, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	printf("waiting for client response...\n");

	ssize_t error = recvfrom(client, data, sizeof data, MSG_WAITALL, (struct sockaddr*)&client_address, &client_length); 
	printf("%s\n", data);

	struct hostent * host = gethostbyaddr(&client_address.sin_addr.s_addr, sizeof(client_address.sin_addr.s_addr), AF_INET);
	char* ip = inet_ntoa(client_address.sin_addr);
	printf("received from %s (%s).\n", host->h_name, ip);

	strncpy(data, "thanks for the response.", sizeof data);
	error = sendto(client, data, sizeof data, 0, (struct sockaddr*)&client_address, client_length);

	close(client);
}
