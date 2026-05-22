#include <time.h>
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


#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

typedef uint64_t nat;
static const nat debug = 0;

#define lightblue "\033[38;5;67m"

#define red   	"\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
#define blue   	"\x1B[34m"
#define magenta "\x1B[35m"
#define cyan   	"\x1B[36m"
#define bold    "\033[1m"
#define reset 	"\x1B[0m"


#define DATA_SIZE (1024 * 1024 * 1024 - 256)




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




static char* read_file(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (not file) {
		fprintf(stderr, bold red "error:" reset bold " ");
		perror(filename);
		fprintf(stderr, reset);
		exit(1);
	}
	fseek(file, 0, SEEK_END);
        size_t length = (size_t) ftell(file); 
	char* text = calloc(length + 1, 1);
        fseek(file, 0, SEEK_SET); 
	fread(text, 1, length, file);
	fclose(file); 

	printf("info: file \"%s\": read %lu bytes\n", filename, length);
	return text;
}






int main(void) {

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


	const char* file_contents = read_file("kernel.cl");


	int err = 0; 
	puts("calling: calloc"); 
	unsigned int* data    = calloc(DATA_SIZE, sizeof(unsigned int));
	if (!data) {
		printf("could not allocate memory using calloc, erroring...\n");
		return 1;
	}
	unsigned int* results = calloc(DATA_SIZE, sizeof(unsigned int));
	if (!results) {
		printf("could not allocate memory using calloc, erroring...\n");
		return 1;
	}

	size_t global = 0;
	size_t local = 0;

	cl_device_id device_id;
	cl_context context;
	cl_command_queue commands; 
	cl_program program;
	cl_kernel kernel; 
	cl_mem input, output;

	puts("calling: (filling up loop with random data/contents...)"); 

	nat count = DATA_SIZE;
	for (nat i = 0; i < count; i++) data[i] = (unsigned int) rand();

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
	
	check_arg(input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(unsigned int) * count, NULL, NULL), input);
	check_arg(output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * count, NULL, NULL), output);

	clock_t begin = clock();
	check(clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(unsigned int) * count, data, 0, NULL, NULL));
	check(clSetKernelArg(kernel, 0, sizeof(cl_mem), &input));
	check(clSetKernelArg(kernel, 1, sizeof(cl_mem), &output));
	check(clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL));

	global = count;

	printf("info: local_group_size = %lu, global_group_size = %lu\n",  local, global);
	check(clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL));
	clFinish(commands);
	check(clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(unsigned int) * count, results, 0, NULL, NULL ));

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("spent %5.5lf seconds on the gpu/opencl...\n", time_spent);

	puts("calling: (verifying results manually...)"); 
	nat correct = 0;
	for (nat i = 0; i < count; i++) {
		if (results[i] == data[i] * data[i]) correct++;
		else {
			printf("incorrect values! expected %u but obtained %u...\n", data[i] * data[i], results[i]);
			break;
		}
	}

	printf("Computed '%llu/%llu' correct values!\n", correct, count);
	puts("calling: clRelease*****"); 

	clReleaseMemObject(input);
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);

	return 0;
}

























/*

Number of platforms                               1
  Platform Name                                   Apple
  Platform Vendor                                 Apple
  Platform Version                                OpenCL 1.2 (Mar  4 2023 12:44:59)
  Platform Profile                                FULL_PROFILE
  Platform Extensions                             cl_APPLE_SetMemObjectDestructor cl_APPLE_ContextLoggingFunctions cl_APPLE_clut cl_APPLE_query_kernel_names cl_APPLE_gl_sharing cl_khr_gl_event

  Platform Name                                   Apple
Number of devices                                 1
  Device Name                                     Apple M1 Max
  Device Vendor                                   Apple
  Device Vendor ID                                0x1027f00
  Device Version                                  OpenCL 1.2 
  Driver Version                                  1.2 1.0
  Device OpenCL C Version                         OpenCL C 1.2 
  Device Type                                     GPU
  Device Profile                                  FULL_PROFILE
  Device Available                                Yes
  Compiler Available                              Yes
  Linker Available                                Yes
  Max compute units                               32
  Max clock frequency                             1000MHz
  Device Partition                                (core)
    Max number of sub-devices                     0
    Supported partition types                     None
    Supported affinity domains                    (n/a)
  Max work item dimensions                        3
  Max work item sizes                             256x256x256
  Max work group size                             256
  Preferred work group size multiple (kernel)     32
  Preferred / native vector sizes                 
    char                                                 1 / 1       
    short                                                1 / 1       
    int                                                  1 / 1       
    long                                                 1 / 1       
    half                                                 0 / 0        (n/a)
    float                                                1 / 1       
    double                                               1 / 1        (n/a)
  Half-precision Floating-point support           (n/a)
  Single-precision Floating-point support         (core)
    Denormals                                     No
    Infinity and NANs                             Yes
    Round to nearest                              Yes
    Round to zero                                 Yes
    Round to infinity                             Yes
    IEEE754-2008 fused multiply-add               Yes
    Support is emulated in software               No
    Correctly-rounded divide and sqrt operations  Yes
  Double-precision Floating-point support         (n/a)
  Address bits                                    64, Little-Endian
  Global memory size                              51539607552 (48GiB)
  Error Correction support                        No
  Max memory allocation                           9663676416 (9GiB)
  Unified memory for Host and Device              Yes
  Minimum alignment for any data type             1 bytes
  Alignment of base address                       32768 bits (4096 bytes)
  Global Memory cache type                        None
  Image support                                   Yes
    Max number of samplers per kernel             32
    Max size for 1D images from buffer            268435456 pixels
    Max 1D or 2D image array size                 2048 images
    Base address alignment for 2D image buffers   256 bytes
    Pitch alignment for 2D image buffers          256 pixels
    Max 2D image size                             16384x16384 pixels
    Max 3D image size                             2048x2048x2048 pixels
    Max number of read image args                 128
    Max number of write image args                8
  Local memory type                               Local
  Local memory size                               32768 (32KiB)
  Max number of constant args                     31
  Max constant buffer size                        1073741824 (1024MiB)
  Max size of kernel argument                     4096 (4KiB)
  Queue properties                                
    Out-of-order execution                        No
    Profiling                                     Yes
  Prefer user sync for interop                    Yes
  Profiling timer resolution                      1000ns
  Execution capabilities                          
    Run OpenCL kernels                            Yes
    Run native kernels                            No
  printf() buffer size                            1048576 (1024KiB)
  Built-in kernels                                (n/a)
  Device Extensions                               cl_APPLE_SetMemObjectDestructor cl_APPLE_ContextLoggingFunctions cl_APPLE_clut cl_APPLE_query_kernel_names cl_APPLE_gl_sharing cl_khr_gl_event cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_3d_image_writes cl_khr_image2d_from_buffer cl_khr_depth_images 

NULL platform behavior
  clGetPlatformInfo(NULL, CL_PLATFORM_NAME, ...)  Apple
  clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, ...)   Success [P0]
  clCreateContext(NULL, ...) [default]            Success [P0]
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_DEFAULT)  Success (1)
    Platform Name                                 Apple
    Device Name                                   Apple M1 Max
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_CPU)  No devices found in platform
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU)  Success (1)
    Platform Name                                 Apple
    Device Name                                   Apple M1 Max
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_ACCELERATOR)  No devices found in platform
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_CUSTOM)  Invalid device type for platform
  clCreateContextFromType(NULL, CL_DEVICE_TYPE_ALL)  Success (1)
    Platform Name                                 Apple
    Device Name                                   Apple M1 Max









*/


