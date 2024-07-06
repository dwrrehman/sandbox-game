/*





202407033.213313:

		


	currently we are doing well in terms of performance-   with a kernel that does 10000 iterations per pixel, and executing it over 3456 * 2234 pixels, ie, 7 million ish, 

				the real time taken by the GPU kernael execution calls        is only  7553 microseconds, which 

										1 / 0.007553    =   132 frames per second, which is more than the refresh rate of this screen, which is 120 lol. so yeah. 


			we are definitely in a good spot in terms of performance, i think, 


			we just now need to get our SDL code into this file, and then figure out how we can pass the write buffer of the pixels data  from SDL, into the write buffer for the kernel's output array, which will in actuality be an array of float4's ie, colors ie pixels. so yeah.  

			pretty interesting! this definitely seems like the optimal way of doing things, i think.   its definitely wayyy more promising than i thought lol. everything is superrrrr easy because its all in C, and really simple to work with lol. 


			lets integrade the sdl code now too.  shouldnt be hard at all lol. 


YAYYY

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

static int window_width = 1600;
static int window_height = 1000;
static float aspect = 1.6f;
static const float fovy = 1.22173f;
static const float znear = 0.01f;
static const float zfar = 1000.0f;
static const float camera_sensitivity = 0.005f;
static const float camera_accel = 0.00003f;
static const float drag = 0.95f;
static const int32_t ms_delay_per_frame = 8;
static bool debug = false;
static bool quit = false;
static bool tab = false;
static bool should_move_camera = true;
static bool is_fullscreen = false;
static int counter = 0;
static float delta = 0.0;
static float pitch = 0.0f, yaw = 0.0f;

struct vec3 { float x,y,z; };

static struct vec3 position = {10, 5, 10};
static struct vec3 velocity = {0, 0, 0};
static struct vec3 forward = 	{0, 0, -1};
static struct vec3 straight = 	{0, 0, 1};
static struct vec3 up = 	{0, 1, 0};
static struct vec3 right = 	{-1, 0, 0};



//static const nat debug = 0;
//#define lightblue "\033[38;5;67m"
//#define red   	"\x1B[31m"
//#define green   "\x1B[32m"
//#define yellow  "\x1B[33m"
//#define blue   	"\x1B[34m"
//#define magenta "\x1B[35m"
//#define cyan   	"\x1B[36m"
//#define bold    "\033[1m"
//#define reset 	"\x1B[0m"
//#define DATA_SIZE (1024 * 1024)

// #define DATA_SIZE (1024 * 1024 * 1024 - 256)         // max possible size.

#define DATA_SIZE (3456 * 2234)         // pixel count on this screen. 



static const char* source_code = 
"__kernel void execute_z_value(__global float* input, __global float* output) {\n"
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
const char *getErrorString(cl_int error) {
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

	const int s = 20;
	const int space_count = s * s * s;
	int8_t* space = (int8_t*) calloc(space_count, 1);

	for (int x = 1; x < s; x++) {
		for (int z = 1; z < s; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = rand() % 2;
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




	// SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

	if (SDL_Init(SDL_INIT_EVERYTHING)) 
		exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow(
		"voxel game", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height, 
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);
	
	SDL_SetRelativeMouseMode(SDL_TRUE);

	//SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);



	SDL_Surface* surface = SDL_GetWindowSurface(window);


	

	printf("surface pixels = %p, format = %s, dimensions = %d x %d.\n", 
		surface->pixels,
		SDL_GetPixelFormatName(surface->format),
		surface->w, surface->h
	);
	//getchar();


	printf("Window pixel format %s\n", SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(window)));
	printf("Surface pixel format %s\n", SDL_GetPixelFormatName(surface->format));






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

	check_arg(kernel = clCreateKernel(program, "execute_z_value", &err), kernel);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create execute_z_value kernel!\n");
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


	bool quit = false;
	while (not quit) {

		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_WINDOWEVENT_RESIZED) {     // <--------- why is this not getting called...!?!?!
				printf("window was resized!!\n");
				int w=0,h=0;
				SDL_GetWindowSize(window, &w, &h);
				printf("width = %d, height = %d", w,h);
				window_width = w;
				window_height = h;
				aspect = (float) window_width / (float) window_height;

				// perspective(perspective_matrix, fovy, aspect, znear, zfar);
			}

			else if (event.type == SDL_MOUSEMOTION and should_move_camera) {
				
    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;
	
				// move_camera();
			}

			else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true; 		// temporary, for debugging. 
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (tab and key[SDL_SCANCODE_0]) debug = !debug;
				if (tab and key[SDL_SCANCODE_1]) { // pause game.
					should_move_camera = not should_move_camera;
					SDL_SetRelativeMouseMode((SDL_bool) should_move_camera);
				}

				if (tab and key[SDL_SCANCODE_2]) {
					is_fullscreen = not is_fullscreen;
					SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					int w = 0, h = 0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;
					// perspective(perspective_matrix, fovy, aspect, znear, zfar);
				}
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);
		
		tab = !!key[SDL_SCANCODE_TAB];

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x += delta * camera_accel * up.x;
			velocity.y += delta * camera_accel * up.y;
			velocity.z += delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			velocity.x -= delta * camera_accel * up.x;
			velocity.y -= delta * camera_accel * up.y;
			velocity.z -= delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			velocity.x += delta * camera_accel * straight.x;
			velocity.y += delta * camera_accel * straight.y;
			velocity.z += delta * camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -= delta * camera_accel * straight.x;
			velocity.y -= delta * camera_accel * straight.y;
			velocity.z -= delta * camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			velocity.x += delta * camera_accel * right.x;
			velocity.y += delta * camera_accel * right.y;
			velocity.z += delta * camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) {
			velocity.x -= delta * camera_accel * right.x;
			velocity.y -= delta * camera_accel * right.y;
			velocity.z -= delta * camera_accel * right.z;
		}

		if (key[SDL_SCANCODE_L]) { 
			yaw -= 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_J]) { 
			yaw += 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_I]) { 
			pitch -= 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_K]) { 
			pitch += 0.08f;
			// move_camera();
		}


		SDL_LockSurface(surface);
		for (nat x = 100; x < 300; x++) {
			for (nat y = 100; y < 300; y++) {
				((uint32_t*)surface->pixels)[(y) * surface->w + (x)] = rand() % 2 == 0 ? 0 : (uint32_t) ~0;
			}
		}
		SDL_UnlockSurface(surface);

		SDL_UpdateWindowSurface(window);

		velocity.x *= drag;
		velocity.y *= drag;
		velocity.z *= drag;

		position.x += delta * velocity.x;
		position.y += delta * velocity.y;
		position.z += delta * velocity.z;

		delta = (float) ((int32_t) SDL_GetTicks() - (int32_t) start);

		nanosleep((const struct timespec[]){{0, 8000000L}}, NULL);

		if (counter == 200) counter = 0;
		else counter++;

		if (counter == 0) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %10.10lf\n", fps);
		}

		if (debug) {
			printf("DEBUG: [%s]\n", tab ? "tab" : "   ");
			printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);
			printf("velocity = {%3.3lf, %3.3lf, %3.3lf}\n", (double)velocity.x,(double)velocity.y,(double)velocity.z);
			printf("yaw = %3.3lf, pitch = %3.3lf\n", (double)yaw, (double)pitch);
			printf("forward = {%3.3lf, %3.3lf, %3.3lf}\n", (double)forward.x,(double)forward.y,(double)forward.z);
			printf("right = {%3.3lf, %3.3lf, %3.3lf}\n", (double)right.x,(double)right.y,(double)right.z);
			printf("up = {%3.3lf, %3.3lf, %3.3lf}\n", (double)up.x,(double)up.y,(double)up.z);
		}	
	}

	clReleaseMemObject(input);
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);

	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
}



































/*


		// matrix stuff?

		auto drawable = swapchain->nextDrawable();
		auto pass = MTL::make_owned(MTL::RenderPassDescriptor::renderPassDescriptor());

		auto color_attachment = pass->colorAttachments()->object(0);
		color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
		color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
		color_attachment->setTexture(drawable->texture());

		const vector_uint2 viewport = { (unsigned int) window_width, (unsigned int) window_height };

		printf("view port = width(x) = %u, height(y) = %u\n", window_width, window_height);

		auto buffer = MTL::make_owned(queue->commandBuffer());
		auto encoder = MTL::make_owned(buffer->renderCommandEncoder(pass.get()));
		encoder->setViewport(MTL::Viewport { 
			(double) window_width / 2.0, (double) window_height / 2.0, 
			(double) window_width, (double) window_height, 
			-0.001, 10000.0
		});
		encoder->setRenderPipelineState(pipeline.get());
		//encoder->setVertexBytes(triangleVertices, sizeof(triangleVertices), 0);
		//encoder->setVertexBytes(&viewport, sizeof(viewport), 1);
		encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), 4);
		encoder->endEncoding();
		buffer->presentDrawable(drawable);
		buffer->commit();
		drawable->release();




















int main(void) { 
	srand((unsigned)time(NULL));

	const int s = 20;
	const int space_count = s * s * s;
	int8_t* space = (int8_t*) calloc(space_count, 1);

	for (int x = 1; x < s; x++) {
		for (int z = 1; z < s; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = rand() % 2;
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


	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	SDL_Window *window = SDL_CreateWindow("block game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
				window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	auto swapchain = (CA::MetalLayer*)SDL_RenderGetMetalLayer(renderer);
	auto device = swapchain->device();
	auto name = device->name();
	printf("using device: \"%s\"...\n", name->utf8String());

	auto library_data = dispatch_data_create(&triangle_metallib[0], triangle_metallib_len, dispatch_get_main_queue(), ^{});
	NS::Error* err;

	auto library = MTL::make_owned(device->newLibrary(library_data, &err));
	if (not library) { printf("Failed to create library"); exit(1); } 

	auto vertex_function_name = NS::String::string("vertexShader", NS::ASCIIStringEncoding);
	auto vertex_function = MTL::make_owned(library->newFunction(vertex_function_name));

	auto fragment_function_name = NS::String::string("fragmentShader", NS::ASCIIStringEncoding);
	auto fragment_function = MTL::make_owned(library->newFunction(fragment_function_name));

	auto pipeline_descriptor = MTL::make_owned(MTL::RenderPipelineDescriptor::alloc()->init());
	pipeline_descriptor->setVertexFunction(vertex_function.get());
	pipeline_descriptor->setFragmentFunction(fragment_function.get());

	auto color_attachment_descriptor = pipeline_descriptor->colorAttachments()->object(0);
	color_attachment_descriptor->setPixelFormat(swapchain->pixelFormat());

	auto pipeline = MTL::make_owned(device->newRenderPipelineState(pipeline_descriptor.get(), &err));
	if (not pipeline) { printf("Failed to create pipeline"); exit(1); } 

	auto queue = MTL::make_owned(device->newCommandQueue());
	

	
	bool quit = false;
	while (not quit) {

		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_WINDOWEVENT_RESIZED) {     // <--------- why is this not getting called...!?!?!
				printf("window was resized!!\n");
				int w=0,h=0;
				SDL_GetWindowSize(window, &w, &h);
				printf("width = %d, height = %d", w,h);
				window_width = w;
				window_height = h;
				aspect = (float) window_width / (float) window_height;

				// perspective(perspective_matrix, fovy, aspect, znear, zfar);
			}

			else if (event.type == SDL_MOUSEMOTION and should_move_camera) {
				
    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;
	
				// move_camera();
			}

			else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true; 		// temporary, for debugging. 
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (tab and key[SDL_SCANCODE_0]) debug = !debug;
				if (tab and key[SDL_SCANCODE_1]) { // pause game.
					should_move_camera = not should_move_camera;
					SDL_SetRelativeMouseMode((SDL_bool) should_move_camera);
				}

				if (tab and key[SDL_SCANCODE_2]) {
					is_fullscreen = not is_fullscreen;
					SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					int w = 0, h = 0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;
					// perspective(perspective_matrix, fovy, aspect, znear, zfar);
				}
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);
		
		tab = !!key[SDL_SCANCODE_TAB];

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x += delta * camera_accel * up.x;
			velocity.y += delta * camera_accel * up.y;
			velocity.z += delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			velocity.x -= delta * camera_accel * up.x;
			velocity.y -= delta * camera_accel * up.y;
			velocity.z -= delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			velocity.x += delta * camera_accel * straight.x;
			velocity.y += delta * camera_accel * straight.y;
			velocity.z += delta * camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -= delta * camera_accel * straight.x;
			velocity.y -= delta * camera_accel * straight.y;
			velocity.z -= delta * camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			velocity.x += delta * camera_accel * right.x;
			velocity.y += delta * camera_accel * right.y;
			velocity.z += delta * camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) {
			velocity.x -= delta * camera_accel * right.x;
			velocity.y -= delta * camera_accel * right.y;
			velocity.z -= delta * camera_accel * right.z;
		}

		if (key[SDL_SCANCODE_L]) { 
			yaw -= 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_J]) { 
			yaw += 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_I]) { 
			pitch -= 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_K]) { 
			pitch += 0.08f;
			// move_camera();
		}


		// matrix stuff?

		auto drawable = swapchain->nextDrawable();
		auto pass = MTL::make_owned(MTL::RenderPassDescriptor::renderPassDescriptor());

		auto color_attachment = pass->colorAttachments()->object(0);
		color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
		color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
		color_attachment->setTexture(drawable->texture());

		const vector_uint2 viewport = { (unsigned int) window_width, (unsigned int) window_height };

		printf("view port = width(x) = %u, height(y) = %u\n", window_width, window_height);

		auto buffer = MTL::make_owned(queue->commandBuffer());
		auto encoder = MTL::make_owned(buffer->renderCommandEncoder(pass.get()));
		encoder->setViewport(MTL::Viewport { 
			(double) window_width / 2.0, (double) window_height / 2.0, 
			(double) window_width, (double) window_height, 
			-0.001, 10000.0
		});
		encoder->setRenderPipelineState(pipeline.get());
		//encoder->setVertexBytes(triangleVertices, sizeof(triangleVertices), 0);
		//encoder->setVertexBytes(&viewport, sizeof(viewport), 1);
		encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), 4);
		encoder->endEncoding();
		buffer->presentDrawable(drawable);
		buffer->commit();
		drawable->release();




		velocity.x *= drag;
		velocity.y *= drag;
		velocity.z *= drag;

		position.x += delta * velocity.x;
		position.y += delta * velocity.y;
		position.z += delta * velocity.z;


		delta = (float) ((int32_t) SDL_GetTicks() - (int32_t) start);

		if (counter == 200) counter = 0;
		else counter++;

		if (counter == 0) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %10.10lf\n", fps);
		}

		if (debug) {
			printf("DEBUG: [%s]\n", tab ? "tab" : "   ");
			printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);
			printf("velocity = {%3.3lf, %3.3lf, %3.3lf}\n", (double)velocity.x,(double)velocity.y,(double)velocity.z);
			printf("yaw = %3.3lf, pitch = %3.3lf\n", (double)yaw, (double)pitch);
			printf("forward = {%3.3lf, %3.3lf, %3.3lf}\n", (double)forward.x,(double)forward.y,(double)forward.z);
			printf("right = {%3.3lf, %3.3lf, %3.3lf}\n", (double)right.x,(double)right.y,(double)right.z);
			printf("up = {%3.3lf, %3.3lf, %3.3lf}\n", (double)up.x,(double)up.y,(double)up.z);
		}	
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


*/


