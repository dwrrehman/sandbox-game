// 1202410093.235441 dwrr
// a 2d game in the terminal
// mulitplayer sandbox game
// to replace the 3d game.

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iso646.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdnoreturn.h>


typedef uint64_t nat;

static const nat side_length = 4000;


static bool displaying = false;
static nat height = 0;
static nat width = 0;
static nat max_length = 0;
static char* screen = NULL;

static nat facing = 0;
static nat breaking = 0;
static nat player_x = 0;
static nat player_y = 0;

static uint32_t* space = NULL;

static struct winsize window = {0};
static struct termios terminal = {0};

static void restore_terminal(void) {
	tcsetattr(0, TCSAFLUSH, &terminal);
	write(1, "\033[?25h", 6);
}

static void configure_terminal(void) {
	tcgetattr(0, &terminal);
	atexit(restore_terminal);
	struct termios raw = terminal;
	raw.c_lflag &= (unsigned long) (~(ECHO | ICANON));
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &raw);
	write(1, "\033[?25l", 6);
}

static void window_resize_handler(int _) {if(_)_++;
	ioctl(0, TIOCGWINSZ, &window); 
	height = window.ws_row - 1; 
	width = window.ws_col - 2; 
	max_length = (size_t) (128 * width * height);
	if (not displaying) screen = realloc(screen, max_length);
}

static noreturn void interrupt_handler(int _) { if(_)_++;
	restore_terminal(); 
	exit(0); 
}

static uint32_t at_player(int64_t x_off, int64_t y_off) {
	const int64_t s = side_length;
	int64_t x = (int64_t) player_x + x_off;
	int64_t y = (int64_t) player_y + y_off;
	x = (x + s) % s;
	y = (y + s) % s;
	return space[y * s + x];
}

static uint32_t* get_at_player(int64_t x_off, int64_t y_off) {
	const int64_t s = side_length;
	int64_t x = (int64_t) player_x + x_off;
	int64_t y = (int64_t) player_y + y_off;
	x = (x + s) % s;
	y = (y + s) % s;
	return space + y * s + x;
}


//static void ____clear_screen(void) { 
	//printf("\033[H\033[2J");
//}

static void display(void) {

	displaying = true;
	nat length = 3;
	memcpy(screen, "\033[H", 3);

	const int64_t h = height / 2;
	const int64_t w = width / 2;

	for (int64_t i = -h; i < h; i++) {
		for (int64_t j = -w; j < w; j++) {


			const uint32_t b = at_player(j, i);
			if (not i and not j) {
				if (facing == 0) screen[length++] = '^';
				if (facing == 1) screen[length++] = '<';
				if (facing == 2) screen[length++] = '>';
				if (facing == 3) screen[length++] = 'v';
			} else if (b == 8) 
				screen[length++] = '#';
			else if (b == 7) 
				screen[length++] = '$';
			else if (b == 6) 
				screen[length++] = '&';
			else if (b == 5) 
				screen[length++] = '%';
			else if (b == 4) 
				screen[length++] = '=';
			else if (b == 3) 
				screen[length++] = '-';
			else if (b == 2) 
				screen[length++] = ',';
			else if (b == 1) 
				screen[length++] = '.';
			else 
				screen[length++] = ' ';
		}
		screen[length++] = 033;
		screen[length++] = '[';
		screen[length++] = 'K';
		screen[length++] = 10;
	}
	write(1, screen, length);
	displaying = false;
}

static void init(void) {
	const nat total = side_length * side_length;
	space = calloc(total, sizeof(uint32_t));
	for (nat i = 0; i < total; i++) {
		space[i] = ((uint32_t) !(rand() % 16)) * 8;
	}
}

static void tick(void) {
	const nat total = side_length * side_length;
	const nat w = (nat) (rand()) % total;
	space[w] = ((uint32_t) !(rand() % 16)) * 8;
}

int main(void) {
	srand((unsigned) time(0));
	window_resize_handler(0);
	struct sigaction action = {.sa_handler = window_resize_handler}; 
	sigaction(SIGWINCH, &action, NULL);
	struct sigaction action2 = {.sa_handler = interrupt_handler}; 
	sigaction(SIGINT, &action2, NULL);
	configure_terminal();
	
	char c = 0;

	init();
loop:	tick();
	display();
	usleep(20000);
	ssize_t n = read(0, &c, 1);
	if (n == 0) goto loop;
	if (n < 0) { perror("read"); exit(1); }
	if (c == 'q') goto done;

	else if (c == 's') {
		facing = 1;
		if (not at_player(-1, 0)) 
			player_x = (player_x + side_length - 1) % side_length;

	} else if (c == 't') {
		facing = 2;
		if (not at_player(1, 0)) 
			player_x = (player_x + side_length + 1) % side_length;

	} else if (c == 'r') {
		facing = 0;
		if (not at_player(0, -1)) 
			player_y = (player_y + side_length - 1) % side_length;

	} else if (c == 'h') {
		facing = 3;
		if (not at_player(0, 1)) 
			player_y = (player_y + side_length + 1) % side_length;
	


	} else if (c == 'n') {
		if (breaking) { if (at_player(-1, 0)) (*get_at_player(-1, 0))--; }
		else {}
		facing = 1;

	} else if (c == 'o') {
		if (breaking) { if (at_player(1, 0)) (*get_at_player(1, 0))--; }
		else {}
		facing = 2;

	} else if (c == 'u') {
		if (breaking) { if (at_player(0, -1)) (*get_at_player(0, -1))--; }
		else {}
		facing = 0;

	} else if (c == 'e') {
		if (breaking) { if (at_player(0, 1)) (*get_at_player(0, 1))--; } 
		else {}
		facing = 3;

	} else if (c == ' ') {
		breaking = not breaking;
	} else {}
	goto loop;
done:
	printf("exiting..\n");
}

/*
copyb insert clang c.c -Weverything -Wno-declaration-after-statement -o run -ferror-limit=1

copya do  ./run




*/






















