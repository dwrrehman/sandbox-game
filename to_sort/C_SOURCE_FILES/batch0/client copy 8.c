// written on 202406156.021136: by dwrr
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
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
//     max size for a usingle ipv6 UDP packet    is:    1,212 bytes.
// 




int main(void) {
	int connection = socket(AF_INET6, SOCK_DGRAM, 0);
	if (connection < 0) { perror("socket"); exit(1); }

	struct sockaddr_in6 address = {0};
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(12000);
	inet_pton(AF_INET6, "2601:1c2:4901:1430:915:d663:e227:ea46", &address.sin6_addr); // ::1 for localhost.
	
	char* message = strdup("HIIII ITS MEE LOLOOL THIS IS COOL");
	ssize_t n = sendto(connection, message, strlen(message), 
		0, (struct sockaddr*) &address, sizeof(address));
	if (n <= 0) { puts("sendto: n <= 0"); abort(); }
	printf("sendto: message sent: %s\n", message);
	
	char reply[1024] = {0};
	socklen_t length = 0;
	puts("client: waiting for reply...");
	n = recvfrom(connection, reply, sizeof(reply) - 1, 
		0, (struct sockaddr*) &address, &length);

	if (n <= 0) { puts("recvfrom: n <= 0"); abort(); }

	char string[INET6_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET6, &(address.sin6_addr), 
		string, sizeof(string));

	printf("info: client received <%s> from %s:%d\n\n", 
		reply, string, ntohs(address.sin6_port)
	);

	close(connection);
}




//	inet6 2601:1c2:4901:1430:1823:9c1c:cc30:57ce prefixlen 64 autoconf secured 
//	inet6 2601:1c2:4901:1430:70b6:bf60:c6ae:bdb2 prefixlen 64 autoconf temporary 
//	inet6 2601:1c2:4901:1430::8c1e prefixlen 64 dynamic 



























/*


	struct sockaddr_in6 server_addr = {0};
	server_addr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, "::1", &server_addr.sin6_addr);
	server_addr.sin6_port = htons(12000);


	struct sockaddr_in6 address = {0};
	address.sin_family = AF_INET;
	address.sin_port = htons(12000);
	address.sin_addr.s_addr = INADDR_ANY;


*/







/*

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

*/

