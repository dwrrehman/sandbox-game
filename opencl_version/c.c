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
	return (struct vec3) {
		x.y * y.z - y.y * x.z,
		x.z * y.x - y.z * x.x,
		x.x * y.y - y.x * x.y
	};
}


#define DATA_SIZE  (3456 * 2234)         // pixel count on this screen. 



static const char* source_code = 
"__kernel void compute_pixel(__global float* input, __global float* output) {\n"
"	int global_id = get_global_id(0);\n"
"	for (int i = 0; i < 10000; i++) {\n"
"		float x = input[global_id] / (float) (i + 1);\n"
"		if (x > 10000.0f) continue;\n"
"	}\n"
"	output[global_id] = input[global_id] * input[global_id];\n"
"	\n"
"}\n"
;



static
const char* getErrorString(cl_int error) {
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







int main(void) {

	srand((unsigned)time(NULL));

	const int s = 100;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	for (int i = 0; i < space_count; i++) {
		space[i] = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 40);
	}

	for (int x = 1; x < 10; x++) {
		for (int z = 1; z < 10; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = 1;
		}
	}

	space[s * s * 1 + s * 1 + 1] = 1;
	space[s * s * 1 + s * 1 + 2] = 1;
	space[s * s * 1 + s * 2 + 1] = 1;
	space[s * s * 1 + s * 2 + 2] = 1;
	space[s * s * 2 + s * 1 + 1] = 1;
	space[s * s * 2 + s * 1 + 2] = 1;
	space[s * s * 2 + s * 2 + 1] = 1;
	space[s * s * 2 + s * 2 + 2] = 1;
	space[s * s * 4 + s * 4 + 4] = 1;

	struct vec3 right =    {1, 0, 0};
	struct vec3 up =       {0, 1, 0};
	struct vec3 straight = {0, 0, 1};
	struct vec3 forward =  {0, 0, 1};
	struct vec3 position = {10, 5, 10};
	struct vec3 velocity = {0, 0, 0};
	float yaw = 0.0f, pitch = 0.0f;

	const float camera_sensitivity = 0.005;
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
	bool resized = true;

		//if (block == 1) { color = (uint32_t) ~0; }
		//if (block == 2) { color = 255; }
		//if (block == 3) { color = 255 << 8; }
		//if (block == 4) { color = 255 << 16; }


	uint32_t colors[256] = {0};

	for (int i = 1; i < 256; i++) {

		const uint32_t R = (rand() % 256U) << 0U;
		const uint32_t G = (rand() % 256U) << 8U;
		const uint32_t B = (rand() % 256U) << 16U;
		const uint32_t A = 255U << 24U;
		colors[i] = R | G | B | A;
	}

	// open cl testing...





	char* value;
	size_t valueSize;

	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;

	// get all platforms

	cl_uint platformCount;
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);

	for (cl_uint i = 0; i < platformCount; i++) {

	unsigned int type = CL_DEVICE_TYPE_ALL;
	clGetDeviceIDs(platforms[i], type, 0, NULL, &deviceCount);
	devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
	clGetDeviceIDs(platforms[i], type, deviceCount, devices, NULL);

	// for each device print critical attributes
	for (cl_uint j = 0; j < deviceCount; j++) {

	// print device name
	clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
	printf("%d. Device: %s\n", j+1, value);
	free(value);

	// print hardware device version
	clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
	printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
	free(value);

	// print software driver version
	clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
	printf(" %d.%d Software version: %s\n", j+1, 2, value);
	free(value);

	// print c version supported by compiler for device
	clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
	printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
	free(value);

	// print parallel compute units
	clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
	sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);

	}
	free(devices);
	}

	free(platforms);
	puts("[finished device info!]\n");


	const char* file_contents = source_code;


	int err = 0; 
	puts("calling: calloc"); 
	float* data    = calloc(DATA_SIZE, sizeof(unsigned int));
	if (!data) {
		printf("could not allocate memory using calloc, erroring...\n");
		return 1;
	}
	float* results = calloc(DATA_SIZE, sizeof(unsigned int));
	if (!results) {
		printf("could not allocate memory using calloc, erroring...\n");
		return 1;
	}

	cl_device_id device_id;
	cl_context context;
	cl_command_queue commands; 
	cl_program program;
	cl_kernel kernel; 
	cl_mem input, output;

	puts("calling: (filling up loop with random data/contents...)"); 

	nat count = DATA_SIZE;
	for (nat i = 0; i < count; i++) data[i] = (float) (rand() % 100);

	check(clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));
	check_arg(context = clCreateContext(0, 1, &device_id, NULL, NULL, &err), context);
	check_arg(commands = clCreateCommandQueue(context, device_id, 0, &err), commands);
	
	check_arg(program = clCreateProgramWithSource(context, 1, (const char **) &file_contents, NULL, &err), program);
    
	check(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
	if (err != CL_SUCCESS) {
		printf("Error: Failed to build program executable!\n");
		size_t len = 0;
		char buffer[2048];
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		printf(" : %s\n", getErrorString(err));
		puts("[press enter to continue]");
		getchar();
		exit(1);
	}

	check_arg(kernel = clCreateKernel(program, "compute_pixel", &err), kernel);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create compute_pixel kernel!\n");
		printf(" : %s\n", getErrorString(err));
		getchar();
	}
	
	check_arg(input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * count, NULL, NULL), input);
	check_arg(output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, NULL), output);


	struct timeval st, et;
	gettimeofday(&st,NULL);


	check(clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * count, data, 0, NULL, NULL));
	check(clSetKernelArg(kernel, 0, sizeof(cl_mem), &input));
	check(clSetKernelArg(kernel, 1, sizeof(cl_mem), &output));

	
	
	size_t local = 32;
	//check(clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL));

	size_t global = count;

	printf("info: local_group_size = %lu, global_group_size = %lu\n",  local, global);
	check(clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL));
	clFinish(commands);
	check(clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL ));

	//clock_t end = clock();
	//double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("spent %5.5lf seconds on the gpu/opencl...\n", time_spent);


	gettimeofday(&et,NULL);
	long elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
	printf("elapsed time: %ld microseconds\n", elapsed);


	//begin = clock();
	puts("calling: (verifying results manually...)"); 
	nat correct = 0;
	for (nat i = 0; i < count; i++) {
		if (results[i] <= data[i] * data[i]) correct++;
		else {
			printf("incorrect values! expected %lf but obtained %lf...\n", (double) (data[i] * data[i]), (double) (results[i]));
			break;
		}
	}

	//end = clock();
	//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("spent %5.5lf seconds verifying on cpu...\n", time_spent);

	printf("Computed '%llu/%llu' correct values!\n", correct, count);
	puts("calling: clRelease"); 







	while (not quit) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) {
				quit = true;
			} else if (event.type == SDL_WINDOWEVENT) {

				if (	event.window.event == SDL_WINDOWEVENT_RESIZED or 
					event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized = true;

			} else if (event.type == SDL_MOUSEMOTION) {

				// printf("motion %u %u : ", event.motion.x, event.motion.y);

				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw += camera_sensitivity * dx;
				pitch -= camera_sensitivity * dy;

				//printf("mouse:    yaw = %lf, pitch = %lf\n", yaw, pitch);
	
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
			//SDL_UpdateWindowSurface(window);
		}

		SDL_LockSurface(surface);

		const int screen_w = surface->w;
		const int screen_h = surface->h;

		const float fov = 0.1f;

		for (int y = 0; y < screen_h; y += 30) {
			for (int x = 0; x < screen_w; x += 30) {

				uint32_t color = 0;

				const float xr = (float) x / (float) screen_w;
				const float yr = (float) y / (float) screen_h;

				const struct vec3 top = cross(forward, right);

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

		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
		nanosleep((const struct timespec[]){{0, 16000000L}}, NULL);

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
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);

	SDL_DestroyWindow(window);
	SDL_Quit();
}
















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






