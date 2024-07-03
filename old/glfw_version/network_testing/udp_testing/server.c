#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(void) {	
	int server = socket(AF_INET, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket creation failed"); exit(1); }
		
	struct sockaddr_in servaddr = {0};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(12000);
		
	int r = bind(server, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (r < 0) { perror("bind failed"); exit(1); }

	char buffer[1024] = {0};
	struct sockaddr_in cliaddr = {0};
	socklen_t length = sizeof(cliaddr);
	puts("server: waiting for message...");
	ssize_t n = recvfrom(server, buffer, sizeof buffer - 1, 0, (struct sockaddr*) &cliaddr, &length);
	if (n <= 0) { puts("recvfrom: n <= 0"); abort(); }
	printf("recvfrom: received message: %s\n", buffer);

	char* message = strdup("my cool reply from server");
	n = sendto(server, message, strlen(message), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
	if (n <= 0) { puts("sendto: n <= 0"); abort(); }
	printf("sendto: message sent: %s\n", message);
}
