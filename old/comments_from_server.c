





































	// for (u64 i = 0, a = 0; i < 100; a = i * i, i++) 
	// 	printf("\rsqrt(%llu) = %llu\n", a, square_root(a));

	// exit(1);




                        
        //wprintf("SERVER:> ");
        //fgets(buffer, sizeof buffer, stdin);
        //n = write(client, buffer, sizeof buffer);
        //if ( n == 0) {
        //    printf("server:write disconnected.\n");
        //    break;
        //}
        //if (!strcmp(buffer, "quit\n")) break;    






/*
 void* handler(void* raw) {
     struct client parameters = *(struct client*)raw;
     int client = parameters.connection;
     const char* ip = parameters.ip;
     
     printf("connected to %s\n", ip);
     
     char buffer[1000] = {0};
     ssize_t n = 0;
     while (server_running) {
         
         memset(buffer, 0, sizeof buffer);
         n = read(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:read disconnected.\n");
             printf("{CLIENT DISCONNECTED}\n");
             break;
         }
         printf("client says: %s\n", buffer);
         
         if (!strcmp(buffer, "halt\n")) {
             printf("halting server...\n");
             halt_server(); continue;
         }
         
         printf("SERVER:> ");
         fgets(buffer, sizeof buffer, stdin);
         n = write(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:write disconnected.\n");
             break;
         }
         if (!strcmp(buffer, "quit\n")) break;
     }
     close(client);
     free(raw);
     return NULL;
 }
 
 */






/*static void connect_to_udp_client(unsigned int port) {

    int connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (!connection) { perror("socket"); return; }
    
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    socklen_t len = sizeof(cliaddr);
    bind(connection, (struct sockaddr*) &servaddr, sizeof(servaddr));
    
    printf("listening for UDP clients...\n");
    
    char buffer[1000] = {0};
        
    while (server_running) {

        memset(buffer, 0, sizeof buffer);
        ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
        if (n == 0) {
            printf("{UDP CLIENT DISCONNECTED}\n");
            break;
        }
        printf("UDP client says: %s\n", buffer);
        
        if (!strcmp(buffer, "halt\n")) {
            printf("halting UDP server...\n");
            halt_server(); continue;
        }

        printf("UDPSERVER:> ");
        fgets(buffer, sizeof buffer, stdin);
        if (!strcmp(buffer, "quit\n")) break;
        sendto(connection, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, len);
    }
    close(connection);
}
*/








// connect_to_udp_client(port); // ?





// static inline noreturn void not_acked() {
// 	printf("debug: error: command not acknowledged from client\n");
// 	abort();
// }




		// } else if (command == ping) {
		// 	printf("SERVER WAS PINGED!!!\n");
		// 	write(client, &ack, 1);

		// } else if (command == chat) {

		// 	memset(buffer, 0, sizeof buffer);
		// 	n = read(client, buffer, sizeof buffer);
		// 	if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
		// 	else if (n < 0) { read_error(); break; }
		// 	printf("client said chat message: %s\n", buffer);
		// 	strcat(transcript, buffer);
		// 	write(client, &ack, 1);









// n = read(client, &response, 1);
			// if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
			// else if (n < 0) { read_error(); break; }
			// if (response != 1) not_acked();




	// while (server_running) {

//		ssize_t n = recvfrom(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
		// if (n == 0) { disconnected(); }

		// printf("UDP client says: %s\n", buffer);

		// if (!strcmp(buffer, "halt\n")) {
		// 	printf("halting UDP server...\n");
		// 	halt_server(); continue;
		// }

		// printf("UDPSERVER:> ");
		// fgets(buffer, sizeof buffer, stdin);
		// if (!strcmp(buffer, "quit\n")) break;

//		sendto(udp_connection, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, len);
	// }






// printf("starting up display thread...\n");
	// pthread_t display_handler_thread;
	// pthread_create(&display_handler_thread, NULL, display_client_handler, &parameters);





// n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len); 
		
// 		printf("debug: sending DP with %d blocks...\n", screen_block_count);

// 		printf("sending block count...\n");
// 		sendto(udp_connection, &screen_block_count, 4, 0, (struct sockaddr*)&cliaddr, len);

// 		// printf("waiting for udp dp ack!\n");
// 		// n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len);
// 		// // check(n); 
// 		// // if (response != 1) not_acked();

// 		printf("sending blocks...\n");
// 		sendto(udp_connection, screen, screen_block_count * 2, 0, (struct sockaddr*)&cliaddr, len);

// 		printf("waiting for udp dp ack!\n");
// 		n = recvfrom(udp_connection, &response, 1, /*MSG_DONTWAIT*/0, (struct sockaddr*)&cliaddr, &len);
// 		check(n); 
// 		// if (response != 1) not_acked();







// static void* display_client_handler(void* raw) {

// 	ssize_t n = 0;
// 	u8 response = 0;

// 	struct client parameters = *(struct client*)raw;

// 	struct player* player = players + parameters.player;
// 	const char* ip = parameters.ip;
// 	u16 port = parameters.port;

// 	printf("arrived in DISPLAY HANDLER: connected to  %s : %d,   player # %d.\n", ip, port + 1, parameters.player);
	
// 	u32 screen_block_count = 0;
// 	u16* screen = malloc(max_block_count * 2);

// 	printf("creating display socket...\n");
// 	int udp_connection = socket(AF_INET, SOCK_DGRAM, 0);
// 	if (!udp_connection) { perror("socket"); abort(); }

// 	struct sockaddr_in servaddr, cliaddr;
// 	memset(&servaddr, 0, sizeof(servaddr));
// 	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	servaddr.sin_port = htons(port + 1);
// 	servaddr.sin_family = AF_INET;
// 	socklen_t len = sizeof(cliaddr);
// 	bind(udp_connection, (struct sockaddr*) &servaddr, sizeof(servaddr));
// 	printf("debug: display client handler: setup udp server on port %d\n", port + 1);

// 	usleep(1000000);
	
// 	printf("debug: display client handler: waiting for client to sendto ACK first...\n");
	
// 	// bool should_continue = 1;
// 	// response = 0;

// 	// while (1) {
// 	// 	n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len); // MSG_DONTWAIT / MSG_WAITALL   MSG_DONTWAIT
// 	// 	if (n > 0) break;
// 	// }

// 	// while (1) {
// 	// 	n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, len);
// 	// 	if (n > 0) break;
// 	// }

// 	printf("debug: in recvfrom, waiting for client to ACK...\n");
// 	usleep(1000);
// 	n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len); 

// 	// if (n > 0) break;

// 	//0 or response != 1) {
// 		// 	printf("error: failed to receive: n = %zd. trying again....\n", n);
// 		// 	should_continue = 0; 
// 		// } else should_continue = 1;
// 		// printf("SENDing...\n");
// 		// u8 ack = 1;
// 		// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*)&cliaddr, len);
// 		// if (n == 0) printf("sendto n = 0\n");
// 		// else if (n < 0) printf("sendto n < 0\n");
// 		// if (should_continue) continue; else break;
// 		// usleep(1000);


// 	printf("debug: display client handler: DONE! received.\n");

// 	while (player->active) {
		
// 		screen_block_count = (rand() % 10000) * 2 + 2;

// 		for (u32 i = 0; i < screen_block_count; i += 2) {
// 			screen[i] = rand() % player->width;
// 			screen[i + 1] = rand() % player->height;
// 		}

// 		// n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len); 
		
// 		printf("debug: sending DP with %d blocks...\n", screen_block_count);

// 		printf("sending block count...\n");
// 		sendto(udp_connection, &screen_block_count, 4, 0, (struct sockaddr*)&cliaddr, len);

// 		// printf("waiting for udp dp ack!\n");
// 		// n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len);
// 		// // check(n); 
// 		// // if (response != 1) not_acked();

// 		printf("sending blocks...\n");
// 		sendto(udp_connection, screen, screen_block_count * 2, 0, (struct sockaddr*)&cliaddr, len);

// 		printf("waiting for udp dp ack!\n");
// 		n = recvfrom(udp_connection, &response, 1, /*MSG_DONTWAIT*/0, (struct sockaddr*)&cliaddr, &len);
// 		check(n); 
// 		// if (response != 1) not_acked();

// 		printf("GOT ACK! done with dp.!\n");

// 		usleep(100000);
// 	}
// 	printf("debug: closing display connection...\n");
// 	close(udp_connection);
// 	free(screen);
// 	return 0;
// }







// static inline void show() {
// 	printf("\nDEBUG: state:  s = %llu, count = %llu\n\n", s, count);
// 	printf("{ \n");
// 	for (u64 i = 0; i < count; i++) {
// 		if (i and (i % 8) == 0) puts("");
// 		printf("%02hhx ", universe[i]);
// 	}
// 	printf("}\n");
// }


