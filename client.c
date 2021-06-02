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


static const u8 colors[] = {
	0,0,0,   	// 0
	255,255,255,    // 1
	255,0,100,   	// 2
	34,34,34,   	// 3
};

enum commands {	
	null_command = 0,
	display = 9, 
	view_resized = 6,
	move_left = 12,
	move_right = 13,
	move_up = 14,
	move_down = 15,
	halt = 255, 
};

static const char* window_title = "universe client";

static int window_height = 400, window_width = 640;
static int scaled_height = 10, scaled_width = 10;
static float scale = 0.02f;

static bool quit = false;

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


static inline void rescale(SDL_Renderer* renderer) {
	scaled_width = (int) ((float)window_width * scale);
	scaled_height = (int) ((float)window_height * scale);
	SDL_RenderSetLogicalSize(renderer, scaled_width, scaled_height);
}

static inline void window_changed(SDL_Window* window, SDL_Renderer* renderer) {
	int w = 0, h = 0;
	SDL_GetWindowSize(window, &w, &h);
	window_width = w;
	window_height = h;
	printf("width and height: (%d, %d)\n", window_width, window_height);
	rescale(renderer);
}

static inline void zoom_in(SDL_Renderer* renderer) {
	int future_scaled_width = (int) ((float)window_width * (scale - 0.005f));
	int future_scaled_height = (int) ((float)window_height * (scale - 0.005f));
	if (not future_scaled_width or not future_scaled_height) return;
	scale -= 0.001f;
	rescale(renderer);
}

static inline void zoom_out(SDL_Renderer* renderer) {
	if (scale >= 0.99f) return;
	scale += 0.001f;
	rescale(renderer);
}

static inline void toggle_fullscreen(SDL_Window* window, SDL_Renderer* renderer) {
	static bool full = false;
	full = !full;
	SDL_SetWindowFullscreen(window, full ? SDL_WINDOW_FULLSCREEN : 0);
	window_changed(window, renderer);
}

static inline void send_resize_command(int connection) {
	u8 command = view_resized, response = 0;
	write(connection, &command, 1);
	write(connection, &scaled_width, 2);
	write(connection, &scaled_height, 2);
	ssize_t n = read(connection, &response, 1); 
	check(n); if (response != 1) not_acked();
}

static inline void send_halt_command(int connection) {
	u8 command = halt, response = 0;
	write(connection, &command, 1);
	ssize_t n = read(connection, &response, sizeof response);
	check(n); if (response != 1) not_acked();
	quit = true; 
}

static inline void send_command(u8 command, int connection) {
	u8 response = 0;
	write(connection, &command, 1);
	ssize_t n = read(connection, &response, 1);
	check(n); if (response != 1) not_acked();
}

int main(const int argc, const char** argv) {
	if (argc != 4) exit(puts("usage: ./client <ip> <port> <playername>"));
	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	const char* ip = argv[1];
	i16 port = (i16) atoi(argv[2]);

	int connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection < 0) { perror("socket"); exit(1); }
	struct sockaddr_in servaddr = {0};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	struct timeval tv;
	tv.tv_sec = 1; // 1 second
	tv.tv_usec = 0;
	setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
	setsockopt(connection, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

	printf("connecting to %s:%d ...\n", ip, port);
	int result = connect(connection, (struct sockaddr*) &servaddr, sizeof servaddr);
	if (result < 0) { perror("connect"); exit(1); }

	printf("\n\n\t %s CONNECTED TO SERVER!\n\n", argv[3]);
	printf("CLIENT[%s:%d]: running...\n", ip, port);

	u8 command = 0, response = 0; 
	ssize_t n = 0;

	u32 screen_block_count = 0;
	const u32 max_block_count = 10000000;
	u16* screen = malloc(max_block_count * 2);
	
	char player_name[30] = {0};
	strncpy(player_name, argv[3], sizeof player_name);

	// printf("sending player name (29 chars)\n");
	write(connection, player_name, 29);
	// printf("receiving ACK for player name..\n");
	n = read(connection, &response, sizeof response);
	check(n); if (response != 1) not_acked();
	
	// printf("sending initial scaled height and width..\n");
	write(connection, &scaled_width, 2);
	write(connection, &scaled_height, 2);
	// printf("receiving ACK for initial height and width!\n");
	n = read(connection, &response, sizeof response);
	check(n); if (response != 1) not_acked();
	
	// printf("FINISHED HANDSHAKE! starting window now...\n");

	SDL_Window *window = SDL_CreateWindow(window_title, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			window_width, window_height, 
			SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_ShowCursor(0);
	window_changed(window, renderer);
	send_resize_command(connection);
	
	while (not quit) {
		uint32_t start = SDL_GetTicks();
		
		command = display;
		write(connection, &command, 1);
		n = read(connection, &screen_block_count, sizeof(u32));
		if (n < 0) printf("DISPLAY ERROR\n");

		u32 local_count = 0;
		while (local_count < screen_block_count) {
			n = read(connection, screen + local_count, 64 * sizeof(u16)); 
			if (n < 0) printf("DISPLAY PACKET ERROR\n");
			// if (n <= 0) continue; // retry this packet. // infinite loop?
			local_count += 64;
		}

		// printf("display: received %d blocks, rendering...\n", screen_block_count);
		int width_radius = (scaled_width - 1) >> 1;
		int height_radius = (scaled_height - 1) >> 1;

		// clear:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    		SDL_RenderClear(renderer);

		// print player:			
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderDrawPoint(renderer, width_radius, height_radius);

		// print space:
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
					send_resize_command(connection);
				}
			}
			
			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_0]) send_halt_command(connection);
				if (key[SDL_SCANCODE_GRAVE]) toggle_fullscreen(window, renderer);
				if (key[SDL_SCANCODE_MINUS]) { zoom_out(renderer); send_resize_command(connection); }
				if (key[SDL_SCANCODE_EQUALS]) { zoom_in(renderer); send_resize_command(connection); }
				
				if (key[SDL_SCANCODE_W]) send_command(move_up, connection);
				if (key[SDL_SCANCODE_A]) send_command(move_left, connection);
				if (key[SDL_SCANCODE_S]) send_command(move_down, connection);
				if (key[SDL_SCANCODE_D]) send_command(move_right, connection);
			}
			if (key[SDL_SCANCODE_SPACE] and key[SDL_SCANCODE_W]) send_command(move_up, connection);
			if (key[SDL_SCANCODE_SPACE] and key[SDL_SCANCODE_A]) send_command(move_left, connection);
			if (key[SDL_SCANCODE_SPACE] and key[SDL_SCANCODE_S]) send_command(move_down, connection);
			if (key[SDL_SCANCODE_SPACE] and key[SDL_SCANCODE_D]) send_command(move_right, connection);
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = 33 - (int32_t) time; 		//16, for 60 fps.
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
	
		if (!(SDL_GetTicks() & 511)) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %.5lf\n", fps);
		}
	}
	close(connection);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	free(screen);
}







// ---------------------------------- dead code -------------------------------------------------------------------------




