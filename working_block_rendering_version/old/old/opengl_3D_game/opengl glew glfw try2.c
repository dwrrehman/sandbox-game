

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// #define GLEW_STATIC

#include <stdbool.h>
#include <iso646.h>
#include <stdnoreturn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static const uint32_t window_width = 1200;
static const uint32_t window_height = 800;
static const char* const window_title = "My cool new game";


noreturn static inline int fail(const char* message) {
	const char* error_string = NULL;
	glfwGetError(&error_string);
	printf("%s : %s\n", message, error_string);
	glfwTerminate(); 
	exit(1);
}

static inline void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
	printf("mouse moved to %lf, %lf\n", xpos, ypos);
}

static inline void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) printf("mouse left button pressed!\n");
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) printf("mouse right button pressed!\n");
}

static inline void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_Q and action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static inline void window_resized_callback(GLFWwindow* window, int width, int height) {
	
}

int main(const int argc, const char** argv) {

	if (not glfwInit()) fail("glfw initialization");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
	if (not window) fail("window");

	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, window_resized_callback);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) fail("glew");

	uint8_t* data = calloc(100 * 100 * 3, 1);
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			data[i * 100 + j * 3 + 0] = 0xff;
			data[i * 100 + j * 3 + 1] = 0x00;
			data[i * 100 + j * 3 + 2] = 0x05;
		}
	}

	while (not glfwWindowShouldClose(window)) {

		//glClear(GL_COLOR_BUFFER_BIT);
		//glDrawPixels(100, 100, GL_RGB, GL_UNSIGNED_BYTE, data);
		glfwSwapBuffers(window);

    		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) printf("W key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) printf("S key was pressed!\n");

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) printf("A key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) printf("D key was pressed!\n");

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) printf("LSHIFT key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) printf("SPACE key was pressed!\n");

		usleep(100000);
	}
	glfwTerminate();
}

