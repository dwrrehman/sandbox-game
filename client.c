//   Client for my multiplayer universe sandbox game.
//        Written by Daniel Warren Riaz Rehman 
//               on 2104305.171454
#include <SDL2/SDL.h>

#include <iso646.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <pwd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;


static const u32 max_block_count = 1 << 16;

// static const u8 ack = 1;

static const u8 colors[] = {
	0,0,0,   // 0
	255,255,255,    // 1
	255,0,100,   // 2
	34,34,34,   // 3
};

enum commands {	
	null_command = 0,
	display = 9, 
	window_resized = 6,
	move_right = 13,
	halt = 255, 
};

static const char* window_title = "universe client";
static int window_height = 800, window_width = 1200;


#define read_error() \
	{do { \
		printf("debug: read error! (n < 0) file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \


#define not_acked() \
	{do { \
		printf("debug: error: command not acknowledged from client file:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \

#define disconnected() \
	{do { \
		printf("debug: error: disconnected:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \


#define check(n) \
	if (n == 0) { disconnected(); } \
	else if (n < 0) { read_error(); } \


static inline void window_changed(SDL_Window* window, SDL_Renderer* renderer) {
	int w = 0, h = 0;
	SDL_GetWindowSize(window, &w, &h);
	SDL_RenderSetLogicalSize(renderer, w, h);
	window_width = w;
	window_height = h;
	printf("width and height: (%d, %d)\n", window_width, window_height);
}

static inline void toggle_fullscreen(SDL_Window* window, SDL_Renderer* renderer) {
	static bool full = false;
	full = !full;
	SDL_SetWindowFullscreen(window, full ? SDL_WINDOW_FULLSCREEN : 0);
	window_changed(window, renderer);
}

int main(const int argc, const char** argv) {
	if (argc != 4) exit(puts("usage: ./client <ip> <port> <playername>"));
	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	

	const char* ip = argv[1];
	i16 port = (i16) atoi(argv[2]);

	int connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection < 0) { perror("socket"); exit(1); }
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);
	printf("connecting to %s:%d ...\n", ip, port);
	int result = connect(connection, (struct sockaddr*) &servaddr, sizeof servaddr);
	if (result < 0) { perror("connect"); exit(1); }

	u8 response = 0;
	char player_name[30] = {0};
	strncpy(player_name, argv[3], sizeof player_name);

	write(connection, player_name, 29);
	ssize_t n = read(connection, &response, sizeof response);
	check(n); if (response != 1) not_acked();

	write(connection, &window_width, 2);
	write(connection, &window_height, 2);
	n = read(connection, &response, sizeof response);
	check(n); if (response != 1) not_acked();

	printf("\n\n\t %s CONNECTED TO SERVER!\n\n", player_name);

	bool quit = false;
	u8 command = 0;

	u32 screen_block_count = 0;
	u16* screen = malloc(max_block_count * 2);

	printf("CLIENT[%s:%d]: running...\n", ip, port);

	int udp_connection = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_connection < 0) { perror("socket"); exit(1); }

	struct sockaddr_in udp_servaddr = {0};
	udp_servaddr.sin_addr.s_addr = inet_addr(ip);
	udp_servaddr.sin_port = htons(port + 1);
	udp_servaddr.sin_family = AF_INET;
	socklen_t len = sizeof(udp_servaddr);

	printf("%s is connecting to UDP server %s : %d...\n", player_name, ip, port + 1);
        
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

	usleep(10);
	n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);
	check(n);
	usleep(10);
	n = sendto(udp_connection, &response, 1, 0, (struct sockaddr*) &udp_servaddr, len);
	check(n);
	usleep(10);
	
	printf("DONE sending ACK to server for UDP con\n");


	// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*)&server_addr, server_struct_length)


	SDL_Window *window = SDL_CreateWindow(window_title, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			window_width, window_height, 
			SDL_WINDOW_RESIZABLE);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_ShowCursor(0);
	
	while (not quit) {
		uint32_t start = SDL_GetTicks();

		printf("receiving block count first...\n");
		n = recvfrom(udp_connection, &screen_block_count, 4, MSG_DONTWAIT, (struct sockaddr*) &udp_servaddr, &len);
		// check(n);

		// printf("sending ACK for bc...\n");
		// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*) &udp_servaddr, len);
		
		printf("receiving %d blocks...\n", screen_block_count);
		n = recvfrom(udp_connection, screen, screen_block_count * 2, MSG_DONTWAIT, (struct sockaddr*) &udp_servaddr, &len);
		// check(n);

		// printf("sending ACK for block array...\n");
		// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*) &udp_servaddr, len);

		// printf("all done!! rendering...\n");

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, colors[4], colors[5], colors[6], 255);

		for (u32 i = 0; i < screen_block_count; i += 2) {
			SDL_RenderDrawPoint(renderer, screen[i], screen[i + 1]);
		}
	    	SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_WINDOWEVENT) {
                		if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					window_changed(window, renderer);
					command = window_resized;
					write(connection, &command, 1);
					write(connection, &window_width, 2);
					write(connection, &window_height, 2);
					n = read(connection, &response, sizeof response); 
					check(n); if (response != 1) not_acked();
				}
			}
			if (event.type == SDL_KEYDOWN) { if (key[SDL_SCANCODE_GRAVE]) toggle_fullscreen(window, renderer); }

			if (event.type == SDL_KEYDOWN) {
				 if (key[SDL_SCANCODE_H]) {
					// SDL_Log("H : halting!\n"); 
					command = halt;
					write(connection, &command, 1);
					n = read(connection, &response, sizeof response);
					check(n); if (response != 1) not_acked();
					quit = true; continue;
				}

				if (key[SDL_SCANCODE_G]) {
					// printf("pressed G! sending display request....\n");

					// printf("receiving block count first...\n");
					// n = recvfrom(udp_connection, &screen_block_count, 4, 0, (struct sockaddr*) &udp_servaddr, &len);
					// check(n);
	
					// printf("sending ACK for bc...\n");
					// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*) &udp_servaddr, len);
					
					// printf("receiving %d blocks...\n", screen_block_count);
					// n = recvfrom(udp_connection, screen, screen_block_count * 2, 0, (struct sockaddr*) &udp_servaddr, &len);
					// check(n);

					// printf("sending ACK for block array...\n");
					// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*) &udp_servaddr, len);

					// printf("all done!!\n");
				}

			}
			if (key[SDL_SCANCODE_ESCAPE]) quit = true;
			if (key[SDL_SCANCODE_Q]) quit = true;

			if (key[SDL_SCANCODE_W]) { SDL_Log("W\n"); }
			if (key[SDL_SCANCODE_S]) { SDL_Log("S\n"); }
			if (key[SDL_SCANCODE_A]) { SDL_Log("A\n"); }

			if (key[SDL_SCANCODE_D]) { 
				// SDL_Log("D : move right\n"); 
				command = move_right;
				write(connection, &command, 1);
				n = read(connection, &response, 1);
				check(n); if (response != 1) not_acked();
			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = 64 - (int32_t) time; //16, for 60 fps.
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
	
		if (!(SDL_GetTicks() & 511)) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %.5lf\n", fps);
		}
	}

	close(connection);
	close(udp_connection);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	free(screen);
}








// sendto(udp_connection, &command, 1, 0, (struct sockaddr*)&cliaddr, len);
// sendto(udp_connection, &ack, 1, 0, (struct sockaddr*)&cliaddr, len);






//        sendto(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, len);

//        ssize_t n = recvfrom(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*) &servaddr, &len);













// ---------------------------------- dead code -------------------------------------------------------------------------







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



