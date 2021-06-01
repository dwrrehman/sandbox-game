



























// sendto(udp_connection, &command, 1, 0, (struct sockaddr*)&cliaddr, len);
// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*)&cliaddr, len);






//        sendto(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, len);

//        ssize_t n = recvfrom(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, &len);
















//if (argc <= 3) return fprintf(stderr, "usage:\n\t ./u <playername> <ipaddress> <port>\n\n");
	// TCP_connect_to_server(argv[1], argv[2], atoi(argv[3]));



// static inline int connect_to_server(const char* address, int port, int* connection) {
// 	printf("info: connecting to \"%s:%d\"...\n", address, port);
// 	*connection = 0;
// 	return 0;
	
// }











/*
static inline void UDP_connect_to_server(const char* playername, const char* ip, unsigned int port) {
    int connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (connection < 0) { perror("socket"); exit(1); }
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    socklen_t len = sizeof(servaddr);
    
    printf("%s is connecting to UDP server...\n", playername);
        
    char buffer[1024] = {0};

    while (1) {
        printf("UDP CLIENT[%s:%d]:> ", ip, port);
        fgets(buffer, sizeof buffer, stdin);
        if (!strcmp(buffer, "quit\n")) break;
        sendto(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, len);

        memset(buffer, 0, sizeof buffer);
        ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, &len);
        if (n == 0) {
            printf("UDP client:read disconnected.\n");
            printf("{UDP SERVER DISCONNECTED}\n");
            break;
        }
        printf("UDP server says: %s\n", buffer);
    }
    close(connection);
}
*/









// for (size_t i = 0; i < count; i += 2) {
		// 	array[i + 0] = (uint16_t) (rand() % window_width);
		// 	array[i + 1] = (uint16_t) (rand() % window_height);
		// }






		// } else if (not strcmp(buffer, "chat\n")) {

		// 	u8 command = chat;
		// 	write(connection, &command, 1);
		// 	printf("message: ");
		// 	fgets(buffer, sizeof buffer, stdin);
		// 	write(connection, buffer, strlen(buffer) + 1);
		// 	ssize_t n = read(connection, &response, sizeof response);
		// 	if (n == 0) { printf("{SERVER DISCONNECTED}\n"); break; }
		// 	else if (n < 0) { read_error(); break; }
			// if (response != 1) not_acked();



// we receive a bloock type, (which determined the block color, and then a count, of that many locations.
		// we do that, for each block type present in the image.





				// else printf("error: unknown command. "
				// "can either be:\n\t"
				// "halt\n\t"
				// "ping\n\t"
				// "display\n\t"
				// "chat\n\t"
				// "quit\n\t"
				// "\n");






// fgets(buffer, sizeof buffer, stdin);





// char buffer[1024] = {0};

    // while (1) {
        // printf("UDP CLIENT[%s:%d]:> ", ip, port);
        // fgets(buffer, sizeof buffer, stdin);
        // if (!strcmp(buffer, "quit\n")) break;






				// if (key[SDL_SCANCODE_G]) {
				// 	SDL_Log("G : send display packet\n"); 
				// 	command = display;
				// 	write(connection, &command, 1);

				// 	n = read(connection, &screen_block_count, 4);
				// 	if (n == 0) { disconnected(); }
				// 	else if (n < 0) { read_error(); }

				// 	n = read(connection, screen, screen_block_count * 2);
				// 	if (n == 0) { disconnected(); }
				// 	else if (n < 0) { read_error(); }
				// }




        // memset(buffer, 0, sizeof buffer);





        // if (n == 0) {
        //     printf("UDP client:read disconnected.\n");
        //     printf("{UDP SERVER DISCONNECTED}\n");
        //     break;
        // }
        // printf("UDP server says: %s\n", buffer);
    // }
    
// }
//





// int udp_connection = socket(AF_INET, SOCK_DGRAM, 0);
	// if (udp_connection < 0) { perror("socket"); exit(1); }

	// struct sockaddr_in udp_servaddr = {0};
	// udp_servaddr.sin_addr.s_addr = inet_addr(ip);
	// udp_servaddr.sin_port = htons(port + 1);
	// udp_servaddr.sin_family = AF_INET;
	// socklen_t len = sizeof(udp_servaddr);

	// printf("%s is connecting to UDP server %s : %d...\n", player_name, ip, port + 1);
        
	// printf("note: about to send ackUDP!!...\n");
	// usleep(10000);

	// printf("sending ACK to server for UDP con\n");

	// while (1) {
		
	// 	if (n == 0) printf("sendto n = 0\n");
	// 	else if (n < 0) printf("sendto n < 0\n");
		
	// 	response = 0;
	// 	// printf("RECVing...\n");
	// 	n = recvfrom(udp_connection, &response, 1, MSG_DONTWAIT, (struct sockaddr*) &udp_servaddr, &len);
	// 	if (n == 0 or n < 0 or response != 1) {
	// 		printf("error: failed to receive: n = %zd. trying again....\n", n);
	// 		continue;
	// 	} else break;
	// }

	// u8 ack = 1;
		// printf("SENDing...\n");


	// while (1) {
	// 	n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);
	// 	if (n > 0) break;
	// }

	// while (1) {
	// 	n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, &len);
	// 	if (n > 0) break;
	// }

	// usleep(100);
	// n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);
	// usleep(100);
	// printf("DONE sending ACK to server for UDP con\n");


	// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*)&server_addr, server_struct_length)







// n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);
		
		// printf("receiving block count first...\n");
		// n = recvfrom(udp_connection, &screen_block_count, 4, 0, (struct sockaddr*) &udp_servaddr, &len);
		// check(n);

		// printf("sending ACK for bc...\n");
		// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*) &udp_servaddr, len);
		
		// printf("receiving %d blocks...\n", screen_block_count);
		// n = recvfrom(udp_connection, screen, screen_block_count * 2, 0, (struct sockaddr*) &udp_servaddr, &len);
		// check(n);

		// printf("sending ACK for block array...\n");
		// response = 1;
		// sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);





// // SDL_Window* window, 
	// int w = window_width + , h = window_height + 1;
	// //if (h > 10000 or w > 10000) return;
	// SDL_RenderSetLogicalSize(renderer, w, h);



// if (h < 2 or w < 2) return;
	// if (h < 2 or w < 2) return;
// window_width = w;
	// window_height = h;
	// printf("width and height: (%d, %d)\n", window_width, window_height);








		// int _i = 0,_j = 0,x_off,y_off;
		// for (_i = 0, x_off = -width_radius; _i < scaled_width; _i++, x_off++) {
		// 	for (_j = 0, y_off = -height_radius; _j < scaled_height; _j++, y_off++) {
		// 		if (not x_off and not y_off) goto double_break_out;
		// 	}
		// }
	
	// double_break_out:
		








