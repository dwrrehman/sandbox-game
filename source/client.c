// UDP client for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <SDL2/SDL.h>

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

static const char* window_title = "universe client";

static int window_height = 400, window_width = 640;
static int scaled_height = 10, scaled_width = 10;
static float scale = 0.02f;
static bool quit = false;

#define check(n) { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

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

int main(const int argc, const char** argv) {
	if (argc != 4) exit(puts("usage: ./client <ip> <port> <playername>"));
	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	const char* ip = argv[1];
	const u16 port = (u16) atoi(argv[2]);
	
	int fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 address = {0};
	address.sin6_family = PF_INET6;
	address.sin6_port = htons(port);
	inet_pton(PF_INET6, ip, &address.sin6_addr); 
	socklen_t size = sizeof address;
	
	u8 response = 0;
	ssize_t error = sendto(fd, "C", 1, 0, (struct sockaddr*) &address, size);
	check(error);

	printf("Connecting to [%s]:%hd ...\n", ip, port);
	error = recvfrom(fd, &response, 1, MSG_DONTWAIT, (struct sockaddr*) &address, &size); 
	check(error);	

	if (response != 'A') return puts("error: connection not acknolwewdged by server.");

	printf("\n\n\t CONNECTED TO SERVER!\n\n");

	u32 screen_block_count = 0;
	const u32 max_block_count = 10000000;
	u16* screen = malloc(max_block_count * 2);

	SDL_Window *window = SDL_CreateWindow(window_title, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			window_width, window_height, 
			SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_ShowCursor(0);
	window_changed(window, renderer);

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		// int width_radius = (scaled_width - 1) >> 1;
		// int height_radius = (scaled_height - 1) >> 1;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    		SDL_RenderClear(renderer);


		// SDL_SetRenderDrawColor(renderer, 67, 45, 234, 255);
		// SDL_RenderDrawPoint(renderer, width_radius, height_radius);

		SDL_SetRenderDrawColor(renderer, colors[4 + 0], colors[4 + 1], colors[4 + 2], 255);
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
					// send_resize_command(connection);
				}
			}
			
			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_0] or key[SDL_SCANCODE_ESCAPE] or key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_0]) {error = sendto(fd, "H", 1, 0, (struct sockaddr*) &address, size); check(error);}

				// if (key[SDL_SCANCODE_GRAVE]) toggle_fullscreen(window, renderer);
				// if (key[SDL_SCANCODE_MINUS]) { zoom_out(renderer); send_resize_command(connection); }
				// if (key[SDL_SCANCODE_EQUALS]) { zoom_in(renderer); send_resize_command(connection); }

				if (key[SDL_SCANCODE_W]) {error = sendto(fd, "w", 1, 0, (struct sockaddr*) &address, size); check(error);}
				if (key[SDL_SCANCODE_A]) {error = sendto(fd, "a", 1, 0, (struct sockaddr*) &address, size); check(error);}
				if (key[SDL_SCANCODE_S]) {error = sendto(fd, "s", 1, 0, (struct sockaddr*) &address, size); check(error);}
				if (key[SDL_SCANCODE_D]) {error = sendto(fd, "d", 1, 0, (struct sockaddr*) &address, size); check(error);}
			}
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
	error = sendto(fd, "D", 1, 0, (struct sockaddr*) &address, size);check(error);
	close(fd);
	free(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

