// written on 202406156.021136: by dwrr
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// note:
//
//    max size of a udp ipv6 packet is  1,212 bytes.
//

 int main(void) {
	int server = socket(AF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); exit(1); }

	struct sockaddr_in6 server_addr = {0};
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(12000);

	int r = bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (r < 0) { perror("bind"); exit(1); }


	for (int i = 0; i < 10; i++) {
		
		char buffer[1024] = {0};

		struct sockaddr_in6 client = {0};
		socklen_t length = sizeof(client);

		puts("server: waiting for message...");
		ssize_t n = recvfrom(server, buffer, sizeof buffer - 1, 
			0, (struct sockaddr*) &client, &length);
		if (n <= 0) { puts("recvfrom: n <= 0"); abort(); }
		
		char string[INET6_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET6, &(client.sin6_addr), 
			string, sizeof(string));

		printf("info: client received <%s> from %s:%d\n\n", 
			buffer, string, ntohs(client.sin6_port)
		);

		char* message = strdup("my cool reply from server");
		n = sendto(server, message, strlen(message), 
			0, (struct sockaddr*) &client, sizeof(client));
		if (n <= 0) { puts("sendto: n <= 0"); abort(); }
		printf("sendto: message sent: %s\n", message);
	}

	close(server);
}








/*
	int server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (server < 0) { perror("socket()"); exit(1); }
 
	//int reuse_address_value = 1;
	//int ret = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse_address_value, sizeof(reuse_address_value));
	//if (ret < 0) { perror("setsockopt()"); exit(1); }

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



printf("info: server received message from:"
		"\n\tipv6addr = %s\n\tport = 12000\n\n", 
		string, ntohs(client.sin6_port));

	printf("recvfrom: received message: %s\n", buffer);






*/

