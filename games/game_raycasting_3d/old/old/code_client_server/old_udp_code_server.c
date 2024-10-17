

























// int connection = socket(AF_INET, SOCK_DGRAM, 0);
//     if (connection < 0) { perror("socket"); exit(1); }
    
//     struct sockaddr_in servaddr;
//     memset(&servaddr, 0, sizeof(servaddr));
//     servaddr.sin_addr.s_addr = inet_addr(ip);
//     servaddr.sin_port = htons(port);
//     servaddr.sin_family = AF_INET;
//     socklen_t len = sizeof(servaddr);
    
//     printf("%s is connecting to UDP server...\n", playername);
        
//     char buffer[1024] = {0};

//     while (1) {
//         printf("UDP CLIENT[%s:%d]:> ", ip, port);
//         fgets(buffer, sizeof buffer, stdin);
//         if (!strcmp(buffer44, "quit\n")) break;
//         sendto(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, len);

//         memset(buffer, 0, sizeof buffer);
//         ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, &len);
//         if (n == 0) {
//             printf("UDP client:read disconnected.\n");
//             printf("{UDP SERVER DISCONNECTED}\n");
//             break;
//         }
//         printf("UDP server says: %s\n", buffer);
//     }
//     close(connection);






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
        if (!strcmp(buffer44, "quit\n")) break;
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










