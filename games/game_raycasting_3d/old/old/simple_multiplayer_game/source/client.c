// UDP client for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <SDL2/SDL.h>

#define check(n) do { if (n == 0 || n < 0) { perror("error occured"); printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); } } while (0)

int main(const int argc, const char** argv) {
	if (argc != 3) exit(puts("usage: ./client <ip> <port>"));

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
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 22, 17);
	SDL_ShowCursor(0);
	
	const int buffer_count = 22 * 17 * 4;

	uint32_t* buffer = malloc(buffer_count);
	uint32_t* pixels = NULL;
	int pitch = 0;
	bool quit = false, fullscreen = false;

	while (not quit) {
		uint32_t start = SDL_GetTicks();
		recvfrom(fd, buffer, buffer_count, MSG_DONTWAIT, (struct sockaddr*) &address, &size); 
		SDL_LockTexture(texture, NULL, (void**) &pixels, &pitch);		
		memcpy(pixels, buffer, buffer_count);
		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_KEYDOWN) {

				if (key[SDL_SCANCODE_TAB]) SDL_SetWindowFullscreen(window,(fullscreen=!fullscreen)?SDL_WINDOW_FULLSCREEN:0);
				if (key[SDL_SCANCODE_ESCAPE] || key[SDL_SCANCODE_GRAVE]) quit = true;
				if (key[SDL_SCANCODE_GRAVE]) sendto(fd, "H", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_RETURN]) sendto(fd, "S", 1, 0, (struct sockaddr*) &address, size);

				if (key[SDL_SCANCODE_A]) sendto(fd, "a", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_B]) sendto(fd, "b", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_C]) sendto(fd, "c", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_D]) sendto(fd, "d", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_E]) sendto(fd, "e", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_F]) sendto(fd, "f", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_G]) sendto(fd, "g", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_H]) sendto(fd, "h", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_I]) sendto(fd, "i", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_J]) sendto(fd, "j", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_K]) sendto(fd, "k", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_L]) sendto(fd, "l", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_M]) sendto(fd, "m", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_N]) sendto(fd, "n", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_O]) sendto(fd, "o", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_P]) sendto(fd, "p", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_Q]) sendto(fd, "q", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_R]) sendto(fd, "r", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_S]) sendto(fd, "s", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_T]) sendto(fd, "t", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_U]) sendto(fd, "u", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_V]) sendto(fd, "v", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_W]) sendto(fd, "w", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_X]) sendto(fd, "x", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_Y]) sendto(fd, "y", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_Z]) sendto(fd, "z", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_0]) sendto(fd, "0", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_1]) sendto(fd, "1", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_2]) sendto(fd, "2", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_3]) sendto(fd, "3", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_4]) sendto(fd, "4", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_5]) sendto(fd, "5", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_6]) sendto(fd, "6", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_7]) sendto(fd, "7", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_8]) sendto(fd, "8", 1, 0, (struct sockaddr*) &address, size);
				if (key[SDL_SCANCODE_9]) sendto(fd, "9", 1, 0, (struct sockaddr*) &address, size);
				
			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = 16 - (int32_t) time; 
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




