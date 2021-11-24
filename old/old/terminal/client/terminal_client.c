// client for the universe sandbox game. 

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef  uint8_t byte;
typedef uint32_t uint;
typedef uint64_t nat;



static nat window_rows = 0;
static nat window_columns = 0;

static bool debug_mode = true;
static char* screen = NULL;
static char message[4096] = {0};


// static byte u[32][32] = {0};
static nat size = 32;


static nat px = 0, py = 0;






static inline struct termios configure_terminal() {
	struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios raw = terminal;
	raw.c_oflag &= ~( (unsigned long)OPOST );
	raw.c_iflag &= ~( (unsigned long)BRKINT 
			| (unsigned long)ICRNL 
			| (unsigned long)INPCK 
			| (unsigned long)IXON );	
	raw.c_lflag &= ~( (unsigned long)ECHO 
			| (unsigned long)ICANON 
			| (unsigned long)IEXTEN );

	raw.c_cc[VMIN] = 0;
  	raw.c_cc[VTIME] = 1;

	tcsetattr(0, TCSAFLUSH, &raw);
	return terminal;
}

static inline void adjust_window_size() {
	struct winsize window = {0};
	ioctl(1, TIOCGWINSZ, &window);

	if (window.ws_row == 0 or window.ws_col == 0) { window.ws_row = 20; window.ws_col = 40; }

	if (window.ws_row != window_rows or window.ws_col != window_columns) {
		window_rows = window.ws_row;
		window_columns = window.ws_col;
		screen = realloc(screen, sizeof(char) * (size_t) (4 * window_rows * (window_columns * 4 + 16)));
	}
}

static inline void display() {
	nat length = 3; 
	memcpy(screen, "\033[H", 3);

	for (nat i = 0; i < size; i++) {
		for (nat j = 0; j < size; j++) {
			if (i == py and j == px) length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 27L, "x ");
			else length += (nat)sprintf(screen + length, "  ");
		}
		screen[length++] = '\033';
		screen[length++] = '[';
		screen[length++] = 'K';
		screen[length++] = '\r';
		screen[length++] = '\n';
	}
	length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 37L, message);
	write(1, screen, (size_t) length);
}

int main(const int argc, const char** argv) {
	
	struct termios terminal = configure_terminal();
	write(1, "\033[?1049h\033[?1000l", 16); // use alternate screen, then disable mouse
	write(1, "\033[?25l", 6); // hide cursor.
	adjust_window_size();

	char c = 0;
	bool quit = false;
	while (not quit) {
		display();
		ssize_t n = read(0, &c, 1);
		if (n == 0) c = 0;
		if (c == 9) quit = true;
		if (c == 'a') adjust_window_size();
		if (c == 'h') strcpy(message, "hello there from space.");
		if (c == 'c') strcpy(message, "");
		if (c == 'j') { if (px) px--; }
		if (c == 'l') { if (px < size) px++; }
		if (c == 'i') { if (py) py--; }
		if (c == 'k') { if (py < size) py++; }
		usleep(10000);
	}

	write(1, "\033[?1049l\033[?1000l", 16);	  // goto main screen, then keep mouse disabled.
	write(1, "\033[?25h", 6); // show cursor again.
	tcsetattr(0, TCSAFLUSH, &terminal);
	
}




























	// const integer height = (integer)window_rows / 2;     
	// const integer width = (integer)window_columns / 4;  

	// for (integer x = -height + 1; x < height; x++) {
	// 	for (integer y = -width; y < width; y++) {

	// 		nat ap = at_player(x,y);
	// 		byte block = universe.state[ap];

	// 		double_break:

	// 		if (not x and not y) {
				

	// 		} else if (in_rogue_path and debug_mode) {
	// 			length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 231L, visualize_rogue_path);
			
	// 		} else if (block == air_block) {
	// 			length += (nat)sprintf(screen + length, "  ");
	// 		} else {
	// 			length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 
	// 						visualize_block_color[block], visualize_block[block]); 
	// 		}
	// 	}
		
	// }

	// const struct player player = universe.players[0];
	// nat status_length = 0, status_visual_length = 0;

	// nat _y = (nat)sprintf(screen + length + status_length, " (%lu, %lu)     \033[38;5;%lum", player.location.x, player.location.y, 9L);
	// status_length += _y;
	// status_visual_length += _y;

	// for (nat i = 0; i < max_player_health; i++) {
	// 	status_length += (nat)sprintf(screen + length + status_length, i < player.health ? "♥ " : "♡ ");
	// 	status_visual_length += 2;
	// }
	// status_length += (nat)sprintf(screen + length + status_length, "\033[m  ");
	// status_visual_length += 3;

	// nat _h = (nat)sprintf(screen + length + status_length, "%s", message);
	// status_length += _h;
	// status_visual_length += _h;

	// length += status_length;

	// for (nat i = status_visual_length; i < window_columns; i++)
	// 	screen[length++] = ' ';

	// screen[length++] = '\033';
	// screen[length++] = '[';
	// screen[length++] = 'm';


