// UDP client for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <SDL2/SDL.h>

#define check(n) { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

int main(const int argc, const char** argv) {
	if (argc != 4) exit(puts("usage: ./client <ip> <port> <playername>"));

	const char* ip = argv[1];
	const uint16_t port = (uint16_t) atoi(argv[2]);
	
	int fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 address = {0};
	address.sin6_family = PF_INET6;
	address.sin6_port = htons(port);
	inet_pton(PF_INET6, ip, &address.sin6_addr); 
	socklen_t size = sizeof address;
	
	uint8_t response = 0;
	ssize_t error = sendto(fd, "C", 1, 0, (struct sockaddr*) &address, size);
	check(error);

	printf("Connecting to [%s]:%hd ...\n", ip, port);
	error = recvfrom(fd, &response, 1, 0, (struct sockaddr*) &address, &size);
	check(error);
	if (response != 'A') return puts("error: connection not acknolwewdged by server.");

	printf("\n\t[connected]\n\n");

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	SDL_Window *window = SDL_CreateWindow("universe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 400, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 10, 10);
	SDL_ShowCursor(0);
	
	const size_t count = 10 * 10;
	uint32_t* buffer = calloc(count, 4); // TODO: make me malloc()
	uint32_t* pixels = NULL;
	int pitch = 0;
	bool quit = false, fullscreen = false;

	while (not quit) {
		uint32_t start = SDL_GetTicks();
		recvfrom(fd, buffer, 10 * 10 * 4, MSG_DONTWAIT, (struct sockaddr*) &address, &size); 
		SDL_LockTexture(texture, NULL, (void**) &pixels, &pitch);		
		memcpy(pixels, buffer, 4 * count);
		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_GRAVE]) SDL_SetWindowFullscreen(window, (fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN : 0);
				if (key[SDL_SCANCODE_0] or key[SDL_SCANCODE_ESCAPE] or key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_0]) sendto(fd, "H", 1, 0, (struct sockaddr*) &address, size);

				if (key[SDL_SCANCODE_W]) sendto(fd, "w", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_A]) sendto(fd, "a", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_S]) sendto(fd, "s", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_D]) sendto(fd, "d", 1, 0, (struct sockaddr*) &address, size);

				if (key[SDL_SCANCODE_T]) sendto(fd, "t", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_F]) sendto(fd, "f", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_G]) sendto(fd, "g", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_H]) sendto(fd, "h", 1, 0, (struct sockaddr*) &address, size);

				if (key[SDL_SCANCODE_I]) sendto(fd, "i", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_J]) sendto(fd, "j", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_K]) sendto(fd, "k", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_L]) sendto(fd, "l", 1, 0, (struct sockaddr*) &address, size);
			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = 33 - (int32_t) time; 
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
	
		if (!(SDL_GetTicks() & 511)) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %.5lf\n", fps);
		}
	}
	sendto(fd, "D", 1, 0, (struct sockaddr*) &address, size);
	close(fd);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}










































// todo     add client side scale back in, to ajust the logical size of the window, both x and y?  idk... 
// x & y are bot independantly changable with the server side width view and height of view. (which should match up with the clients logical height and view?.... hmmm thats interesting..
// we can make it so that they are always insync? maybe? except for packet loss... :( owell...




























/*
		the idea is to send a resize command byte, and then a 5 byte packet of    { continutation_byte, w=u16, h=u16 }.

			thats it!
		just make sure that when we receive the 5 byte packet, that it says a continutation byte,    to verify its correct. 

		also make sure that the command receiver ignores the continutation byte packets. 

			thaats it!





s
*/









// ------------------------------------------------ work in progress:---------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static inline void zoom_in(SDL_Renderer* renderer) {
// 	int future_scaled_width = (int) ((float)window_width * (scale - 0.005f));
// 	int future_scaled_height = (int) ((float)window_height * (scale - 0.005f));
// 	if (not future_scaled_width or not future_scaled_height) return;
// 	scale -= 0.001f;
// 	rescale(renderer);
// }

// static inline void zoom_out(SDL_Renderer* renderer) {
// 	if (scale >= 0.99f) return;
// 	scale += 0.001f;
// 	rescale(renderer);
// }



// static inline void send_resize_command(int connection) {
// 	u8 command = view_resized, response = 0;
// 	write(connection, &command, 1);
// 	write(connection, &scaled_width, 2);
// 	write(connection, &scaled_height, 2);
// 	ssize_t n = read(connection, &response, 1); 
// 	check(n); if (response != 1) not_acked();
// }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// static void interrupt_kill(int __attribute__((unused)) _) {
// 	kill(getpid(), SIGINT);
// }	
























		// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  //   		SDL_RenderClear(renderer);

		// // SDL_SetRenderDrawColor(renderer, 67, 45, 234, 255);
		// // SDL_RenderDrawPoint(renderer, width_radius, height_radius);

		// SDL_SetRenderDrawColor(renderer, colors[3 + 0], colors[3 + 1], colors[3 + 2], 255);
		// for (u32 i = 0; i < screen_block_count; i += 2) {
		// 	SDL_RenderDrawPoint(renderer, screen[i], screen[i + 1]);
		// }





 // , SDL_Renderer* renderer, SDL_Texture** texture
// SDL_DestroyTexture(*texture);
	// *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);


// static inline void toggle_fullscreen(SDL_Window* window) { // , SDL_Renderer* renderer, SDL_Texture** texture
	
// }


