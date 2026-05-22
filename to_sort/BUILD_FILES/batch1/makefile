# makefile for the 256 game. written by daniel rehman, on 2008296.215814.

server: source/server/server/main.c
	gcc -O3 -o server source/server/server/main.c

client: source/client/client/main.cpp
	g++ -std=c++17 -O3 -o client -I/usr/local/Cellar/glm/0.9.9.5/include/ -I/usr/local/Cellar/glew/2.1.0_1/include/ -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/ -I/usr/local/include/ source/client/client/main.cpp -L/usr/local/Cellar/glew/2.1.0_1/lib -lGLEW -L/usr/local/Cellar/sdl2/2.0.12_1/lib -L/usr/local/Cellar/sdl2_ttf/2.0.15/lib -lSDL2_ttf-2.0.0 -lSDL2-2.0.0 -framework OpenGL


