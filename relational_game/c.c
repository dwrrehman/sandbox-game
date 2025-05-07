// a relational type of game, where the world is constructed via connections alone! possibly turn based, idk. 
// written by dwrr on 1202505073.014443

#include <signal.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>

typedef uint64_t nat;
static nat height = 0;
static nat width = 0;
static nat max_length = 0;
static char* screen = NULL;
static uint32_t* space = NULL;
static struct winsize window = {0};
static struct termios terminal = {0};

static char* read_file(const char* name) {
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) {  perror("open"); exit(1); } 
	const size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = calloc(length + 1, 1);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file);
	return string;
}

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
	if (displaying) return;
	ioctl(0, TIOCGWINSZ, &window); 
	height = 30;	//window.ws_row - 10; 
	width = 80;	//window.ws_col - 20; 
	max_length = (size_t) (128 * width * height);
	screen = realloc(screen, max_length);
}

int main (void) {
	srand((unsigned) time(0));
	window_resize_handler(0);
	struct sigaction action = {.sa_handler = window_resize_handler}; 
	sigaction(SIGWINCH, &action, NULL);
	struct sigaction action2 = {.sa_handler = interrupt_handler}; 
	sigaction(SIGINT, &action2, NULL);
	configure_terminal();
	char c = 0;
	generate_world();
loop:	tick();
	display();
	usleep(10000);
	ssize_t n = read(0, &c, 1);
	if (n == 0) goto loop;
	if (n < 0) { perror("read"); exit(1); }
	if (c == 'q') goto done;	
	else if (c == 't') { }
	else {}
	goto loop;
done:	puts("done");
}









/*
1202505073.014839
	
	trying to figure out what might be the design of the game! 

	i'm thinking it could be something where a list of connections are displayed to you, 
	and then you need to form the dimensional structure in your head  in order to see what the world looks like lolll
		idk, i feel like things would definitely need to be turn based if we were to go with that lolll 
		hmm


	on the other hand, i kinda want things to be interactive in a similar manner to how real life is interactive lol. and like, how you are able to very intuitively just intreact with the world... idk... hmmmmmm


		the problem is,  making things 3D is quite difficult kinda.. idk... 



		at least, in the terminal it definitely is lol 


	hmmmm

	those are the two ways i want to go with though. one of those lol.  either a turn based   relational  approach  where we just list connections that are the case, 

	orrr a real time interactive experience where the world actually happens around you   and you are able to interact with it in real time.   hmmm

		its interesting lollll













*/






