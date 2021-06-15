



















// setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	// setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));



// setsockopt(connection, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	// setsockopt(connection, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));




	// int connection = server;
	// int connection = socket(PF_INET6, SOCK_DGRAM, 0);
	// if (connection < 0) { perror("socket"); abort(); }

	// int result = bind(connection, (struct sockaddr*) &address, length);
	// if (result < 0) { perror("bind"); abort(); }

	// printf("info: client-hanlder's connection running on  [%s]:%hu \n", ip, port);






// static void* client_handler(void* raw) {

// 	struct client_data parameters = *(struct client_data*)raw;
// 	struct sockaddr_in6 address = parameters.address;
// 	socklen_t length = parameters.length;

// 	u8 command = 0;

// 	char ip[40] = {0};
// 	ipv6_string(ip, address.sin6_addr.s6_addr);
// 	printf("server: connected to IP = %s\n", ip);

// 	bool client_running = true;

// 	while (server_running and client_running) {
		
// 		printf("client[%s] : ", ip);
// 		printf("reading command...\n");
		
// 		ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);		
// 		if (error == 0) { printf("DISCONNECTED!\n"); break; }
// 		else check(error);

// 		if (command == 'N') { 
// 			printf("client[%s] : ", ip);
// 			printf("no operation.\n");
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		}

// 		else if (command == 'H') { 
// 			printf("client[%s] : ", ip);
// 			printf("SERVER: halting...\n"); 
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);

// 			server_running = false;
// 			shutdown(server, SHUT_RDWR); 
// 			close(server);
// 		}

// 		else if (command == 'D') { 
// 			printf("client[%s] : ", ip);
// 			printf("info: client sent a disconnection request.\n"); 
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 			client_running = false; 
// 		}

// 		else if (command == 'P') {
// 			printf("client[%s] : ", ip);
// 			printf("server was PINGED!!!\n");
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		} 

// 		else {
// 			printf("client[%s] : ", ip);
// 			printf("warning: received unknown commmand: %c\n", command);
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		}
// 	}

// 	printf("client[%s] : ", ip);
// 	printf("debug: leaving client handler...\n");
// 	free(raw);
// 	return 0;
// }





		// if (response != 'C') {
		// 	printf("intercepted random message... dropping...\n");
		// 	continue;
		// }




		// struct client_data* client_data = malloc(sizeof(struct client_data));
		// client_data->address = address;
		// client_data->length = len;
		
		// printf("starting handler thread for connection...\n");
		// pthread_t handler_thread;
		// pthread_create(&handler_thread, NULL, client_handler, client_data);
		// pthread_detach(handler_thread);
		// usleep(10000);

	// 	struct client_data parameters = *(struct client_data*)raw;
	// struct sockaddr_in6 address = parameters.address;
	// socklen_t length = parameters.length;





		// ipv6_string(ip, address.sin6_addr.s6_addr);
		// printf("server: connected to IP = %s\n", ip);

			// printf("client[%s] : ", ip);
			// printf("reading command...\n");
			
			// ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);		
			// if (error == 0) { printf("DISCONNECTED!\n"); break; }
			// else check(error);






// struct client_data {
// 	struct sockaddr_in6 address;
// 	socklen_t length;
// };



// error = sendto(server, "@", 1, 0, (struct sockaddr*)&address, length);
		// check(error);



// printf("client [%s]: --> they said: %c\n", ip, command);