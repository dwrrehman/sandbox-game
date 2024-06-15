#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(void) {
	int connection = socket(AF_INET, SOCK_DGRAM, 0);
	if (connection < 0) { perror("socket creation failed"); exit(1); }

	struct sockaddr_in address = {0};
	address.sin_family = AF_INET;
	address.sin_port = htons(12000);
	address.sin_addr.s_addr = INADDR_ANY;
	
	char* message = strdup("Hello from client");
	ssize_t n = sendto(connection, message, strlen(message), 0, (struct sockaddr*) &address, sizeof(address));
	if (n <= 0) { puts("sendto: n <= 0"); abort(); }
	printf("sendto: message sent: %s\n", message);
	
	char reply[1024] = {0};
	socklen_t length = 0;
	puts("client: waiting for reply...");
	n = recvfrom(connection, reply, sizeof(reply) - 1, 0, (struct sockaddr*) &address, &length);
	if (n <= 0) { puts("recvfrom: n <= 0"); abort(); }
	printf("recvfrom: server's reply = %s\n", reply);
	close(connection);
}
