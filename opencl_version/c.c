/*  202407055.232919:   dwrr   

 opencl-based voxel sandbox 3d game.

    still a work in progress!

*/
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <SDL.h>

typedef uint64_t nat;
struct vec3 { float x, y, z; };

static inline float inversesqrt(float y) {
	float x2 = y * 0.5f;
	int32_t i = *(int32_t *)&y;
	i = 0x5f3759df - (i >> 1); 	// glm uses a86 for last three digits.
	y = *(float*) &i;
	return y * (1.5f - x2 * y * y);
}

static inline struct vec3 normalize(struct vec3 v) {
	float s = inversesqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return (struct vec3) {v.x * s, v.y * s, v.z * s};
}

static inline struct vec3 cross(struct vec3 x, struct vec3 y) {
	return (struct vec3) { x.y * y.z - y.y * x.z, x.z * y.x - y.z * x.x, x.x * y.y - y.x * x.y };
}


static const char* getErrorString(cl_int error) {
switch(error) {
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}

#define CaseReturnString(x) case x: return #x;

static const char *opencl_errstr(cl_int err) {
    switch (err) {
        CaseReturnString(CL_SUCCESS                        )                                  
        CaseReturnString(CL_DEVICE_NOT_FOUND               )
        CaseReturnString(CL_DEVICE_NOT_AVAILABLE           )
        CaseReturnString(CL_COMPILER_NOT_AVAILABLE         ) 
        CaseReturnString(CL_MEM_OBJECT_ALLOCATION_FAILURE  )
        CaseReturnString(CL_OUT_OF_RESOURCES               )
        CaseReturnString(CL_OUT_OF_HOST_MEMORY             )
        CaseReturnString(CL_PROFILING_INFO_NOT_AVAILABLE   )
        CaseReturnString(CL_MEM_COPY_OVERLAP               )
        CaseReturnString(CL_IMAGE_FORMAT_MISMATCH          )
        CaseReturnString(CL_IMAGE_FORMAT_NOT_SUPPORTED     )
        CaseReturnString(CL_BUILD_PROGRAM_FAILURE          )
        CaseReturnString(CL_MAP_FAILURE                    )
        CaseReturnString(CL_MISALIGNED_SUB_BUFFER_OFFSET   )
        CaseReturnString(CL_COMPILE_PROGRAM_FAILURE        )
        CaseReturnString(CL_LINKER_NOT_AVAILABLE           )
        CaseReturnString(CL_LINK_PROGRAM_FAILURE           )
        CaseReturnString(CL_DEVICE_PARTITION_FAILED        )
        CaseReturnString(CL_KERNEL_ARG_INFO_NOT_AVAILABLE  )
        CaseReturnString(CL_INVALID_VALUE                  )
        CaseReturnString(CL_INVALID_DEVICE_TYPE            )
        CaseReturnString(CL_INVALID_PLATFORM               )
        CaseReturnString(CL_INVALID_DEVICE                 )
        CaseReturnString(CL_INVALID_CONTEXT                )
        CaseReturnString(CL_INVALID_QUEUE_PROPERTIES       )
        CaseReturnString(CL_INVALID_COMMAND_QUEUE          )
        CaseReturnString(CL_INVALID_HOST_PTR               )
        CaseReturnString(CL_INVALID_MEM_OBJECT             )
        CaseReturnString(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        CaseReturnString(CL_INVALID_IMAGE_SIZE             )
        CaseReturnString(CL_INVALID_SAMPLER                )
        CaseReturnString(CL_INVALID_BINARY                 )
        CaseReturnString(CL_INVALID_BUILD_OPTIONS          )
        CaseReturnString(CL_INVALID_PROGRAM                )
        CaseReturnString(CL_INVALID_PROGRAM_EXECUTABLE     )
        CaseReturnString(CL_INVALID_KERNEL_NAME            )
        CaseReturnString(CL_INVALID_KERNEL_DEFINITION      )
        CaseReturnString(CL_INVALID_KERNEL                 )
        CaseReturnString(CL_INVALID_ARG_INDEX              )
        CaseReturnString(CL_INVALID_ARG_VALUE              )
        CaseReturnString(CL_INVALID_ARG_SIZE               )
        CaseReturnString(CL_INVALID_KERNEL_ARGS            )
        CaseReturnString(CL_INVALID_WORK_DIMENSION         )
        CaseReturnString(CL_INVALID_WORK_GROUP_SIZE        )
        CaseReturnString(CL_INVALID_WORK_ITEM_SIZE         )
        CaseReturnString(CL_INVALID_GLOBAL_OFFSET          )
        CaseReturnString(CL_INVALID_EVENT_WAIT_LIST        )
        CaseReturnString(CL_INVALID_EVENT                  )
        CaseReturnString(CL_INVALID_OPERATION              )
        CaseReturnString(CL_INVALID_GL_OBJECT              )
        CaseReturnString(CL_INVALID_BUFFER_SIZE            )
        CaseReturnString(CL_INVALID_MIP_LEVEL              )
        CaseReturnString(CL_INVALID_GLOBAL_WORK_SIZE       )
        CaseReturnString(CL_INVALID_PROPERTY               )
        CaseReturnString(CL_INVALID_IMAGE_DESCRIPTOR       )
        CaseReturnString(CL_INVALID_COMPILER_OPTIONS       )
        CaseReturnString(CL_INVALID_LINKER_OPTIONS         )
        CaseReturnString(CL_INVALID_DEVICE_PARTITION_COUNT )
        default: return "Unknown OpenCL error code";
    }
}

#define check(statement) \
	do {\
		printf("opencl: calling: ");\
		puts(#statement);\
		err = statement;\
		if (err != CL_SUCCESS) {\
			printf("opencl: error:   ");\
			puts(#statement);\
        		printf("Error number: %d", err);\
	        	printf(" : %s (\"%s\")\n", getErrorString(err), opencl_errstr(err));\
			puts("[press enter to continue]");\
			getchar();\
		}\
	} while(0);

#define check_arg(statement, condition) \
	do {\
		printf("opencl: calling: ");\
		puts(#statement);\
		statement;\
		if (!condition) {\
			printf("opencl: error:   ");\
			puts(#statement);\
        		printf("Error number: %d", err);\
	        	printf(" : %s (\"%s\")\n", getErrorString(err), opencl_errstr(err));\
			puts("[press enter to continue]");\
			getchar();\
		}\
	} while(0);


#define qcheck(statement) \
	do {\
		err = statement;\
		if (err != CL_SUCCESS) {\
			printf("opencl: error:   ");\
			puts(#statement);\
        		printf("Error number: %d", err);\
	        	printf(" : %s (\"%s\")\n", getErrorString(err), opencl_errstr(err));\
			puts("[press enter to continue]");\
			getchar();\
		}\
	} while(0);




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



int main(void) {
	//srand((unsigned)time(NULL));

	srand(42);
	const int s = 500;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	const int modulus = 10;
	const int density = 64;

	for (int i = 0; i < space_count; i++) 
		space[i] = rand() % density == 0 ? rand() % modulus : 0;
	
	for (int x = 1; x < 10; x++) {
		for (int z = 1; z < 10; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = 1;
		}
	}

	struct vec3 position = {10, 5, 10};
	struct vec3 velocity = {0, 0, 0};	
	struct vec3 right =    {1, 0, 0};
	struct vec3 up =       {0, 1, 0};
	struct vec3 forward =  {0, 0, 1};
	struct vec3 straight = cross(right, up);
	struct vec3 top = cross(forward, right);
	float yaw = 0.0f, pitch = 0.0f;
	const float camera_sensitivity = 0.005f;
	const float pi_over_2 = 1.57079632679f;
	const float camera_accel = 0.05f;

	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("init: %s\n", SDL_GetError()));
	int window_width = 1600, window_height = 1000;
	SDL_Window *window = SDL_CreateWindow(
		"voxel game", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	SDL_SetRelativeMouseMode(1);
	bool quit = false;
	bool resized = false;
	uint32_t frame_counter = 0;

	char* value;
	size_t valueSize;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;
	cl_uint platformCount;
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);
	for (cl_uint i = 0; i < platformCount; i++) {
	unsigned int type = CL_DEVICE_TYPE_ALL;
	clGetDeviceIDs(platforms[i], type, 0, NULL, &deviceCount);
	devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
	clGetDeviceIDs(platforms[i], type, deviceCount, devices, NULL);
	for (cl_uint j = 0; j < deviceCount; j++) {
	clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
	printf("%d. Device: %s\n", j+1, value);
	free(value);
	clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
	printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
	free(value);
	clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
	printf(" %d.%d Software version: %s\n", j+1, 2, value);
	free(value);
	clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
	printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
	free(value);
	clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
	sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);
	} free(devices);
	}
	free(platforms);
	int err = 0; 
	cl_device_id device_id;
	cl_context context;
	cl_command_queue commands; 
	cl_program program;
	cl_kernel kernel; 
	cl_mem input, output, space_array;

	check(clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));
	check_arg(context = clCreateContext(0, 1, &device_id, NULL, NULL, &err), context);
	check_arg(commands = clCreateCommandQueue(context, device_id, 0, &err), commands);
	char* source_code = read_file("kernel.cl");
	check_arg(program = clCreateProgramWithSource(context, 1, (const char **) &source_code, NULL, &err), program);
	check(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
	if (err != CL_SUCCESS) {
		printf("Error: Failed to build program executable!\n");
		size_t len = 0;
		char buffer[2048];
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		printf(" : %s\n", getErrorString(err));
		exit(1);
	}

	check_arg(kernel = clCreateKernel(program, "compute_pixel", &err), kernel);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create compute_pixel kernel!\n");
		printf(" : %s\n", getErrorString(err));
		exit(1);
	}

	const size_t data_size = 15;
	float data[15] = {0};

	SDL_GetWindowSize(window, &window_width, &window_height);
	surface = SDL_GetWindowSurface(window);
	size_t pixel_count = (size_t) surface->w * (size_t) surface->h;
	check_arg(input =       clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * data_size, NULL, NULL), input);
	check_arg(space_array = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(uint8_t) * space_count, NULL, NULL), space_array);
	check_arg(output =      clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(uint32_t) * pixel_count, NULL, NULL), output);
	data[0] = (float) surface->w;
	data[1] = (float) surface->h;
	data[2] = (float) s;

	if (pitch > pi_over_2) pitch = pi_over_2 - 0.0001f;
	else if (pitch < -pi_over_2) pitch = -pi_over_2 + 0.0001f;
	forward.x = -sinf(yaw) * cosf(pitch);
	forward.y = -sinf(pitch);
	forward.z = -cosf(yaw) * cosf(pitch);
	forward = normalize(forward);
	right.x = -cosf(yaw);
	right.y = 0.0;
	right.z = sinf(yaw);
	right = normalize(right);
	straight = cross(right, up);
	top = cross(forward, right);
	data[6] = right.x;
	data[7] = right.y;
	data[8] = right.z;
	data[9] = top.x;
	data[10] = top.y;
	data[11] = top.z;
	data[12] = forward.x;
	data[13] = forward.y;
	data[14] = forward.z;


	float average_fps = 0.0;

	while (not quit) {
		struct timeval st, et;
		gettimeofday(&st, NULL);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) { quit = true; }
			else if (event.type == SDL_WINDOWEVENT) {
				if (	event.window.event == SDL_WINDOWEVENT_RESIZED or 
					event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized = true;

			} else if (event.type == SDL_MOUSEMOTION) {

				const float dx = (float) event.motion.xrel;
    				const float dy = (float) event.motion.yrel;
				yaw += camera_sensitivity * dx;
				pitch -= camera_sensitivity * dy;
				if (pitch > pi_over_2) pitch = pi_over_2 - 0.0001f;
				else if (pitch < -pi_over_2) pitch = -pi_over_2 + 0.0001f;
				forward.x = -sinf(yaw) * cosf(pitch);
				forward.y = -sinf(pitch);
				forward.z = -cosf(yaw) * cosf(pitch);
				forward = normalize(forward);
				right.x = -cosf(yaw);
				right.y = 0.0;
				right.z = sinf(yaw);
				right = normalize(right);
				straight = cross(right, up);
				top = cross(forward, right);
				data[6] = right.x;
				data[7] = right.y;
				data[8] = right.z;
				data[9] = top.x;
				data[10] = top.y;
				data[11] = top.z;
				data[12] = forward.x;
				data[13] = forward.y;
				data[14] = forward.z;

			} else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_Q]) quit = true; 
				if (key[SDL_SCANCODE_Z]) SDL_SetRelativeMouseMode(0);
				if (key[SDL_SCANCODE_G]) SDL_SetRelativeMouseMode(1);
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x -= camera_accel * up.x;
			velocity.y -= camera_accel * up.y;
			velocity.z -= camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			velocity.x += camera_accel * up.x;
			velocity.y += camera_accel * up.y;
			velocity.z += camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			velocity.x += camera_accel * straight.x;
			velocity.y += camera_accel * straight.y;
			velocity.z += camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -= camera_accel * straight.x;
			velocity.y -= camera_accel * straight.y;
			velocity.z -= camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			velocity.x -= camera_accel * right.x;
			velocity.y -= camera_accel * right.y;
			velocity.z -= camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) {
			velocity.x += camera_accel * right.x;
			velocity.y += camera_accel * right.y;
			velocity.z += camera_accel * right.z;
		}

		if (resized) {
			resized = 0;
			SDL_GetWindowSize(window, &window_width, &window_height);
			printf("Resizing, now: window_width = %d,  window_height = %d\n", 
				window_width, window_height
			);
			surface = SDL_GetWindowSurface(window);
			pixel_count = (size_t)surface->w * (size_t)surface->h;
			clReleaseMemObject(output);
			check_arg(output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(uint32_t) * pixel_count, NULL, NULL), output);

			data[0] = (float) surface->w;
			data[1] = (float) surface->h;
		}
		data[3] = position.x;
		data[4] = position.y;
		data[5] = position.z;

		qcheck(clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * data_size, data, 0, NULL, NULL));
		qcheck(clEnqueueWriteBuffer(commands, space_array, CL_TRUE, 0, sizeof(uint8_t) * space_count, space, 0, NULL, NULL));
		qcheck(clSetKernelArg(kernel, 0, sizeof(cl_mem), &input));
		qcheck(clSetKernelArg(kernel, 1, sizeof(cl_mem), &output));
		qcheck(clSetKernelArg(kernel, 2, sizeof(cl_mem), &space_array));
		
		SDL_LockSurface(surface);
		size_t local = 1;
		qcheck(clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &pixel_count, &local, 0, NULL, NULL));
		clFinish(commands);
		qcheck(clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(uint32_t) * pixel_count, surface->pixels, 0, NULL, NULL));
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);

		gettimeofday(&et,NULL);
		long elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);

		
		if (frame_counter >= 10) { 
			frame_counter = 0; 
			float fps = 1.0f / ((float) elapsed / 1000000.0f);

			average_fps = average_fps * 0.8 + fps * 0.2;

			printf("%lf:  elapsed time: %ld microseconds : average_fps = %lf\n", average_fps, elapsed, average_fps); 

		} else frame_counter++; 



		nanosleep((const struct timespec[]){{0, (8000000L - elapsed * 1000 > 0 ? 16000000L - elapsed * 1000 : 0)}}, NULL);

		velocity.x *= 0.96f;
		velocity.y *= 0.96f;
		velocity.z *= 0.96f;
		position.x += velocity.x;
		position.y += velocity.y;
		position.z += velocity.z;
		position.x = fmodf(position.x + (float)s, (float)s);
		position.y = fmodf(position.y + (float)s, (float)s);
		position.z = fmodf(position.z + (float)s, (float)s);
	}
	clReleaseMemObject(input);
	clReleaseMemObject(space_array);
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}















































	/*			raw raycasting cpu algorithm:




		const int screen_w = surface->w;
		const int screen_h = surface->h;

		const float fov = 0.1f;

	
		for (int y = 0; y < screen_h; y += 30) {
			for (int x = 0; x < screen_w; x += 30) {

				uint32_t color = 0;

				const float xr = (float) x / (float) screen_w;
				const float yr = (float) y / (float) screen_h;

				struct vec3 step = forward;
				step.x /= 10;
				step.y /= 10;
				step.z /= 10;

				const float st_x = -fov + 2 * fov * xr;
				const float st_y = -fov + 2 * fov * yr;
				
				step.x += top.x * st_y;
				step.y += top.y * st_y;
				step.z += top.z * st_y;

				step.x += right.x * st_x;
				step.y += right.y * st_x;
				step.z += right.z * st_x;

				struct vec3 ray = position;

				for (int n = 0; n < 100; n++) {      


					// NEXT STEP:    DO   the DDA ALGORITHM  don't step 0.1 per 
					//             ray step, step a whole block, always!!! nice. 


					// thennnn think about doing gpu stuff. 
					// honestly we could even just make this more efficient by other means first, though. 
					// doing it on the gpu is probably the last step. yay. 

					int px = (int) ray.x;
					int py = (int) ray.y;
					int pz = (int) ray.z;
					
					const int8_t block = space[s * s * pz + s * py + px];
					
					if (block) {
						color = colors[block];
						break;
					}

					ray.x = fmodf(ray.x + step.x + (float)s, (float)s);
					ray.y = fmodf(ray.y + step.y + (float)s, (float)s);
					ray.z = fmodf(ray.z + step.z + (float)s, (float)s);
				}

				for (int i = 0; i < 5; i++) {
					if (y + i >= screen_h) continue;
					for (int j = 0; j < 5; j++) {
						if (x + j >= screen_w) break;
						((uint32_t*)surface->pixels)[(y + i) * surface->w + (x + j)] = color;
					}
				}
			}
		}
	*/






// size_t global = pixel_count;
		//printf("info: global_group_size = %lu\n",  pixel_count);


			//} else if (event.type == SDL_WINDOWEVENT_FULLSCREEN) {

			//	SDL_Surface* surface = SDL_GetWindowSurface(window);
			//	printf("window was resized!!\n");





				//	printf("window was resized!!\n");

				// todo: handle all of even.window.event equal to  
				///  SDL_WINDOWEVENT_RESIZED or 
				//	SDL_WINDOWEVENT_SIZE_CHANGED or 
				//	SDL_WINDOWEVENT_MAXIMIZED or 
				//	SDL_WINDOWEVENT_RESTORED

				//SDL_Surface* surface = SDL_GetWindowSurface(window);

				/*	SDL_GetWindowSize(window, &window_width, &window_height);
					printf("Resizing, now: window_width = %d,  window_height = %d\n", 
						window_width, window_height
					);

					// SDL_SetWindowSize(window, width, height);
					SDL_FreeSurface(surface);
					surface = SDL_GetWindowSurface(window);
					SDL_BlitSurface(image,NULL,surface,NULL);
					SDL_UpdateWindowSurface(window);
					}
				*/






/*




	w = 3456 
	
	h = 2234



	2200 * 3456 + 3450 = 7606650


	7606650 % 3456 = x   ==    3,450

	(7606650 / 3456)  ==   2,200.9982638889   which when considerd as an integer gives you   2200 



	nice, so thast the computation to make a global_id(0) value   and turn it into the x and y coordinates, which are used for creating the notion of pixels   and ray directions. 



	
 














//clock_t end = clock();
	//double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("spent %5.5lf seconds on the gpu/opencl...\n", time_spent);


	
	//begin = clock();
	//puts("calling: (verifying results manually...)"); 
	//nat correct = 0;
	//for (nat i = 0; i < count; i++) {
	//	if (results[i] <= data[i] * data[i]) correct++;
	//	else {
	//		printf("incorrect values! expected %lf but obtained %lf...\n", (double) (data[i] * data[i]), (double) (results[i]));
	//		break;
	//	}
	//}

	//end = clock();
	//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("spent %5.5lf seconds verifying on cpu...\n", time_spent);



		//if (block == 1) { color = (uint32_t) ~0; }
		//if (block == 2) { color = 255; }
		//if (block == 3) { color = 255 << 8; }
		//if (block == 4) { color = 255 << 16; }







		puts("calling: (verifying results manually...)"); 
		nat correct = 0;
		for (nat i = 0; i < count; i++) {
			if (results[i] <= data[i] * data[i]) correct++;
			else {
				printf("incorrect values! expected %lf but obtained %lf...\n", (double) (data[i] * data[i]), (double) (results[i]));
				break;
			}
		}
		printf("Computed '%llu/%llu' correct values!\n", correct, count);
		



//check(clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL));





	puts("calling: (filling up loop with random data/contents...)"); 

	// nat count = DATA_SIZE;
	// for (nat i = 0; i < count; i++) data[i] = (float) (rand() % 100);






*/





// #define DATA_SIZE  (3456 * 2234)         // pixel count on this screen. 


/*


"	int id = get_global_id(0);\n"
"	const float w = input[0];\n"
"	const float h = input[1];\n"
"	const unsigned int wi = (unsigned int) w;\n"
"	const unsigned int hi = (unsigned int) h;\n"
//"	printf(\"%u %u\\n\", wi, hi);\n"
"	output[id] = wi == 3200 ? (id % 65536) : 0;\n"



	7606650 % 3456 = x   ==    3,450

	(7606650 / 3456)  ==   2,200.9982638889   which when considerd as an integer gives you   2200 







		//step.x += top.x * st_y;
	//step.y += top.y * st_y;
	//step.z += top.z * st_y;

	

	//step.x += right.x * st_x;
	//step.y += right.y * st_x;
	//step.z += right.z * st_x;

	//struct vec3 ray = position;
// NEXT STEP:    DO   the DDA ALGORITHM  don't step 0.1 per 
		//             ray step, step a whole block, always!!! nice. 


		// thennnn think about doing gpu stuff. 
		// honestly we could even just make this more efficient by other means first, though. 
		// doing it on the gpu is probably the last step. yay. 



	const unsigned int id = get_global_id(0);
	const float w = input[0];
	const float h = input[1];
	const float sf = input[2];
	const unsigned int wi = (unsigned int) w;
	const unsigned int hi = (unsigned int) h;
	const unsigned int s = (unsigned int) sf;
	const float x = (float)(id % wi);
	const float y = (float)(id / wi);
	const float xr = x / w;
	const float yr = y / h;
	
	float3 ray   = (float3) (input[3], input[4], input[5]);
	float3 right = (float3) (input[6], input[7], input[8]);
	float3 top   = (float3) (input[9], input[10], input[11]);
	float3 step = (float3) (input[12], input[13], input[14]);
	
	const float fov = 0.3;
	const float st_x = -fov + 2 * fov * xr;
	const float st_y = -fov + 2 * fov * yr;

	step *= 0.1;
	step += top * st_y;
	step += right * st_x;

	for (unsigned int n = 0; n < 100; n++) {
		const unsigned int px = (unsigned int) ray.x;
		const unsigned int py = (unsigned int) ray.y;
		const unsigned int pz = (unsigned int) ray.z;
		const unsigned char block = space[s * s * pz + s * py + px];
		
		if (block) {			
			output[id] = 0xdeadbeef;
			return;
		}
		ray = fmod(ray + step + sf, sf);		
	}

	
	







// color = colors[block];



		//ray.x = fmodf(ray.x + step.x + (float)s, (float)s);
		//ray.y = fmodf(ray.y + step.y + (float)s, (float)s);
		//ray.z = fmodf(ray.z + step.z + (float)s, (float)s);






	w = 3456 
	
	h = 2234



	2200 * 3456 + 3450 = 7606650


	7606650 % 3456 = x   ==    3,450

	(7606650 / 3456)  ==   2,200.9982638889   which when considerd as an integer gives you   2200 



	nice, so thast the computation to make a global_id(0) value   and turn it into the x and y coordinates, which are used for creating the notion of pixels   and ray directions. 









static const char* source_code = 
"__kernel void compute_pixel(__global unsigned int* input, __global unsigned int* output, __global const unsigned char* space) {\n"
"	int id = get_global_id(0);\n"
"	for (int i = 0; i < 10000; i++) {\n"
"		unsigned int x = input[id] / (unsigned int) (i + 1);\n"
"		if (x > 10000.0f) continue;\n"
"	}\n"
"	output[id] = input[id] * input[id];\n"
"	\n"
"}\n"
;



*/






/*#define qcheck_arg(statement, condition) \
	do {\
		statement;\
		if (!condition) {\
			printf("opencl: error:   ");\
			puts(#statement);\
        		printf("Error number: %d", err);\
	        	printf(" : %s (\"%s\")\n", getErrorString(err), opencl_errstr(err));\
			puts("[press enter to continue]");\
			getchar();\
		}\
	} while(0);
*/








	/*uint32_t colors[256] = {0};
	for (int i = 1; i < 256; i++) {
		const uint32_t R = ((uint32_t) rand() % 256U) << 0U;
		const uint32_t G = (rand() % 256U) << 8U;
		const uint32_t B = (rand() % 256U) << 16U;
		const uint32_t A = 255U << 24U;
		colors[i] = R | G | B | A;
	}*/







	//puts("calling: calloc"); 




	//float* results = calloc(DATA_SIZE, sizeof(unsigned int));
	//if (!results) {
	//	printf("could not allocate memory using calloc, erroring...\n");
	//	return 1;
	//}




				// printf("motion %u %u : ", event.motion.x, event.motion.y);
				//printf("mouse:    yaw = %lf, pitch = %lf\n", yaw, pitch);


//  printf("Resizing, now: window_width = %d,  window_height = %d\n", window_width, window_height);






/*







static const char* source_code = 
"__kernel void compute_pixel(__constant float* input, __global unsigned int* output, __global unsigned char* space) {\n"
"\n"
"	const unsigned int id = get_global_id(0);\n"
"	const float w = input[0];\n"
"	const float h = input[1];\n"
"	const float sf = input[2];\n"
"	const unsigned int wi = (unsigned int) w;\n"
"	const unsigned int hi = (unsigned int) h;\n"
"	const unsigned int s = (unsigned int) sf;\n"
"	const float x = (float)(id % wi);\n"
"	const float y = (float)(id / wi);\n"
"	const float xr = x / w;\n"
"	const float yr = y / h;\n"
"\n"
"	float3 ray   = (float3) (input[3], input[4], input[5]);\n"
"	float3 right = (float3) (input[6], input[7], input[8]);\n"
"	float3 top   = (float3) (input[9], input[10], input[11]);\n"
"	float3 step = (float3) (input[12], input[13], input[14]);\n"
"\n"
"	const float fov = 0.25;\n"
"	const float st_x = -fov + 2 * fov * xr;\n"
"	const float st_y = -fov + 2 * fov * yr;\n"
"\n"
"	step *= 0.15;\n"
"	step += top * st_y;\n"
"	step += right * st_x;\n"
"\n"
"	unsigned short a = 0xFF;\n"
"	unsigned short r = 0x00;\n"
"	unsigned short b = 0x00;\n"
"	unsigned short g = 0x00;\n"
"	for (unsigned int n = 0; n < 2000; n++) {\n"
"		const unsigned int px = (unsigned int) ray.x;\n"
"		const unsigned int py = (unsigned int) ray.y;\n"
"		const unsigned int pz = (unsigned int) ray.z;\n"
"		const unsigned char block = space[s * s * pz + s * py + px];\n"
"\n"
"		if (block) {\n"
"			     if (block == 1) { r += 0x2F; g += 0x00; b += 0x00; }\n"
"			else if (block == 2) { r += 0xcc; g += 0x00; b += 0x00; break; }\n"
"			else if (block == 3) { r += 0x00; g += 0xcc; b += 0x00; break; }\n"
"			else if (block == 4) { r += 0x00; g += 0x00; b += 0xcc; break; }\n"
"			else if (block == 5) { r += 0xcc; g += 0xcc; b += 0x00; break; }\n"
"			else if (block == 6) { r += 0x00; g += 0xcc; b += 0xcc; break; }\n"
"			else if (block == 7) { r += 0xcc; g += 0x00; b += 0xcc; break; }\n"
"			else if (block == 8) { r += 0x0c; g += 0x0c; b += 0xcc; break; }\n"
"			else if (block == 9) { r += 0x0c; g += 0xcc; b += 0x0c; break; }\n"
"			else                 { r += 0xFF; g += 0xFF; b += 0xFF; break; }\n"
"			if (r >= 0xFF) r = 0xFF;\n"
"			if (g >= 0xFF) g = 0xFF;\n"
"			if (b >= 0xFF) b = 0xFF;\n"
"		}\n"
"		next: ray = fmod(ray + step + sf, sf); continue;\n"
"	}\n"
"	if (r >= 0xFF) r = 0xFF;\n"
"	if (g >= 0xFF) g = 0xFF;\n"
"	if (b >= 0xFF) b = 0xFF;\n"
"	const unsigned int color = (a << 24) | (r << 16) | (g << 8) | (b << 0);\n"
"	output[id] = color;\n"
"\n"
"}\n";






*/














	/* 











to compute the distance which we advance in the ray
	upon moving one unit on the x axis, is given by S_x, 
	which is defined to be:
		S_x = sqrt(1 + (dy/dx) * (dy/dx) )
	the y axis has something similar:
		S_y = sqrt(1 + (dx/dy) * (dx/dy) )
	we move this amount, when we choose that particular axis
	to move along. furthermore, 


wait crap this is all in two dimensions lololol 



oopsies 



hmm



the distance of the hypotenuse is:

	S = sqrt(dx^2 + dy^2 + dz^2)

in 3D. this means that there are three S_# variables, 

	S_x = sqrt(1 + (dy/dx)^2 + (dz/dx)^2)
	S_y = sqrt((dx/dy)^2 + 1 + (dz/dy)^2)
	S_y = sqrt((dx/dz)^2 + (dy/dz)^2 + 1)

so now, we know the amount to 

okay so basically we know that  

	the  "step" variable    step.x      is dx 

		step.y  is dy   and step.z  is dz 

	thus, step.x / step.y     is dx/dy     

	thus now, i think we know how to 
		construct these S_# variables. 


	yay


so given these, 

	we now create top level local variables, called

	rx,  ry   rz      which are the ray progress 
				in each of the axies. 


question:
we initialize these to the   noninteger or floating point part 
	of the "ray" variable (which of course, is our starting position for every ray)..


		is this the case?



	

	







heres some sudo code i found online:


	float3 raydir = normalize(mousecell - player);
	float S_x = sqrt(1 + (step.y/step.x)*(step.y/step.x) + (step.z/step.x)*(step.z/step.x));
	float S_y = sqrt((step.x/step.y)*(step.x/step.y) + 1 + (step.z/step.y)*(step.z/step.y));
	float S_z = sqrt((step.x/step.z)*(step.x/step.z) + (step.y/step.z)*(step.y/step.z) + 1);
	float3 rayunitstepsize = { S_x, S_y, S_z, };

	float3 raystart = position;
	int3 mapcheck = (int3) raystart;
	float3 raylength1d = {0};

	int3 unit = {0};

	if (raydir.x < 0) {
		unit.x = -1;
		raylength.x = (raystart.x - (float)(mapcheck.x)) * rayunitstepsize.x;
	} else {
		unit.x = 1;
		raylength.x = ((float)(mapcheck.x + 1) - raystart.x) * rayunitstepsize.x;
	}

	if (raydir.y < 0) {
		unit.y = -1;
		raylength.y = (raystart.y - (float)(mapcheck.y)) * rayunitstepsize.y;
	} else {
		unit.y = 1;
		raylength.y = ((float)(mapcheck.y + 1) - raystart.y) * rayunitstepsize.y;
	}

	for (int i = 0; i < 1000; i++) {

		if (raylength1d.x <  raylength1d.y) {
			mapcheck.x += unit.x;
			raylength1d.x += rayunitstepsize.x;
		} else {
			mapcheck.y += unit.y;
			raylength1d.y += rayunitstepsize.y;
		}

		uint8 block = space[mapcheck.xyz];
		if (block) {
			color = colors[block];
			goto found;
		}
	}
	color = black;
	found: printf("hello");


	
	*/


// "	printf(\"%lf, %lf, %lf\\n\", S_x, S_y, S_z);\n"

// //"	step *= 0.15;\n"








