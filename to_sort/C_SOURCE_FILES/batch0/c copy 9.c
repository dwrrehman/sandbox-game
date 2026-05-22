// a program to split a file in half or put two halfs together, which is useful for allowing me to push a particularly large binary file to git.. lol. this program shouldnt exist, and git lfs is kinda not helping. so yeah. 
// written on 1202407033.011547 by dwrr

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>

int main(const int argc, const char** argv) {

	if (argc != 5) return puts(
		"usage: ./form <many/one> file0 file1 file2"
	);
	
	const int flags = O_WRONLY 
		  | O_APPEND 
		  | O_EXCL 
		  | O_CREAT;

	const mode_t mode = S_IRUSR 
		    | S_IWUSR 
		    | S_IRGRP 
		    | S_IROTH;

	if (not strcmp(argv[1], "many")) {
		const char* dest0_name = argv[2];
		const char* dest1_name = argv[3];
		const char* source0_name = argv[4];

		puts("opening source 0");
		int fd_source0 = open(source0_name, O_RDONLY);
		if (fd_source0 < 0) { perror("open"); exit(1); }

		puts("opening dest 0");
		int fd_dest0 = open(dest0_name, flags, mode);
		if (fd_dest0 < 0) { perror("open"); exit(1); }

		puts("opening dest 1");
		int fd_dest1 = open(dest1_name, flags, mode);
		if (fd_dest1 < 0) { perror("open"); exit(1); }

		const size_t length = lseek(fd_source0, 0, SEEK_END);
		lseek(fd_source0, 0, SEEK_SET);
		char* contents = calloc(length, 1);
		read(fd_source0, contents, length);
		printf("read %llu bytes from source: %s\n", 
			length, source0_name
		);
		const size_t p = length >> 1;

		printf("writing @%p : %llub to %s...",
			(void*)(contents), p, dest0_name
		);
		write(fd_dest0, contents, p);

		printf("writing @%p : %llub to %s...",
			(void*)(contents + p), length - p, dest1_name
		);
		write(fd_dest1, contents + p, length - p);

		close(fd_dest1);
		close(fd_dest0);
		close(fd_source0);

	} else if (not strcmp(argv[1], "one")) {

		const char* dest0_name = argv[2];
		const char* source0_name = argv[3];
		const char* source1_name = argv[4];

		puts("opening source 0");
		int fd_source0 = open(source0_name, O_RDONLY);
		if (fd_source0 < 0) { perror("open"); exit(1); }

		puts("opening source 1");
		int fd_source1 = open(source1_name, O_RDONLY);
		if (fd_source1 < 0) { perror("open"); exit(1); }

		puts("opening dest 0");
		int fd_dest0 = open(dest0_name, flags, mode);
		if (fd_dest0 < 0) { perror("open"); exit(1); }

		const size_t length0 = lseek(fd_source0, 0, SEEK_END);
		lseek(fd_source0, 0, SEEK_SET);
		char* contents0 = calloc(length0, 1);
		read(fd_source0, contents0, length0);
		printf("read %llu bytes from source 0: %s\n", 
			length0, source0_name
		);

		const size_t length1 = lseek(fd_source1, 0, SEEK_END);
		lseek(fd_source1, 0, SEEK_SET);
		char* contents1 = calloc(length1, 1);
		read(fd_source1, contents1, length1);
		printf("read %llu bytes from source 1: %s\n", 
			length1, source1_name
		);

		printf("writing @%p : %llub to %s...",
			(void*)(contents0), length0, source0_name
		);
		write(fd_dest0, contents0, length0);

		printf("writing @%p : %llub to %s...",
			(void*)(contents1), length1, source1_name
		);
		write(fd_dest0, contents1, length1);

		close(fd_source1);
		close(fd_source0);
		close(fd_dest0);
	} else {
		puts("usage error: unknown function.");
		abort();
	}
	puts("done.");
}






