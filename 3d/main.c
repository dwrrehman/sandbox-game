#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <iso646.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdnoreturn.h>

#define window_width 640
#define window_height 480
#define window_title "My cool new game"




static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
                                                     uint64_t object, size_t location, int32_t message_code,
                                                     const char *layer_prefix, const char *message, void *user_data)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		printf("\033[0;31mValidation Layer: Error\033[m: %s: %s\n", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		printf("\033[0;33mValidation Layer: Warning\033[m: %s: %s\n", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		printf("\033[0;35mValidation Layer: Performance warning\033[m: %s: %s\n", layer_prefix, message);
	}
	else
	{
		printf("\033[0;36mValidation Layer: Information\033[m: %s: %s\n", layer_prefix, message);
	}
	return VK_FALSE;
}








#define get(name) \
	PFN_##name name = (PFN_##name) glfwGetInstanceProcAddress(NULL, #name)



noreturn static inline void vk_error(const char* function) {
	printf("vulkan:  %s  : error\n", function);
	exit(1);
}

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



int main(const int argc, const char** argv) {

	printf("initilaizing glfw...\n");
	if (not glfwInit()) fail("glfw initialization");

	printf("checking for vulkan support...\n");
	if (not glfwVulkanSupported()) fail("vulkan not supported");
	
	printf("creating the window...\n");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
	if (not window) fail("window");

	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	printf("loading vulkan bingings...\n");
	get(vkCreateInstance);
	get(vkEnumeratePhysicalDevices);
	get(vkEnumerateDeviceExtensionProperties);
	get(vkGetPhysicalDeviceQueueFamilyProperties);
	get(vkCreateDevice);


	const char* extensions[] = {
		"VK_KHR_surface", 
		"VK_EXT_metal_surface", 
		"VK_EXT_validation_features", 
		"VK_KHR_get_physical_device_properties2"
	};

	const char* validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
	VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};

	VkApplicationInfo application = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "My Cool Game Application",
		.pEngineName      = "Game Engine Name Here",
		.apiVersion       = VK_MAKE_VERSION(1, 0, 0)
	};

	VkDebugReportCallbackCreateInfoEXT debug_report_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
		.flags = 
			VK_DEBUG_REPORT_ERROR_BIT_EXT | 
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT | 
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | 
			VK_DEBUG_REPORT_WARNING_BIT_EXT,
		.pfnCallback = debug_callback
	};
	
	VkValidationFeaturesEXT features = {
		.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
		.pNext = &debug_report_info,
 		.enabledValidationFeatureCount = 1,
		.pEnabledValidationFeatures = enables
	};

	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &features,
		.pApplicationInfo = &application,
		.enabledExtensionCount = 4, 
		.ppEnabledExtensionNames = extensions,
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = validation_layers
	};

	printf("debug: creating instance...\n");
	VkInstance instance;
	if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS) 
		vk_error("create instance");
	

	printf("debug: enumerating the physical devices...\n");
	uint32_t physical_device_count = 5;
	VkPhysicalDevice physical_devices[5] = {0};
	if (vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices) != VK_SUCCESS) 
		vk_error("enumerate physical devices");

	if (not physical_device_count) vk_error("no suitable GPU physical device found");
	VkPhysicalDevice physical_device = physical_devices[0];

	printf("debug: getting the queues...\n");

	uint32_t queue_family_count = 10;
	VkQueueFamilyProperties queue_family_properties[10] = {0};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

	printf("debug: finding queue family index...\n");
	uint32_t queue_index = 0;
	for (; queue_index < queue_family_count; queue_index++) {
		if (glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_index)) 
			break;
	}

	if (queue_index == queue_family_count) 
		vk_error("no suitable queue family index found for the device");
	
	printf("debug: using queue_family_index = %d    (out of %d)\n", queue_index, queue_family_count);



	uint32_t property_count = 100;
	VkExtensionProperties properties[100] = {0};
	if (vkEnumerateDeviceExtensionProperties(physical_device, NULL, &property_count, properties) != VK_SUCCESS) 
		vk_error("enumerate device extension properties");


	const char* device_extensions[] = {"VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	float queue_priority = 1.0;
	VkDeviceQueueCreateInfo queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = queue_index,
		.queueCount       = 1,
		.pQueuePriorities = &queue_priority,
	};

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = device_extensions,
	};

	printf("debug: creating the device...\n");
	VkDevice device;
	if (vkCreateDevice(physical_device, &device_info, NULL, &device) != VK_SUCCESS) 
		vk_error("create device");


	printf("debug: getting the queue...\n");
	VkQueue queue;
	vkGetDeviceQueue(device, queue_index, 0, &queue);


	printf("debug: creating window surface...\n");
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (err) fail("window surface");

	while (not glfwWindowShouldClose(window)) {

   		glfwSwapBuffers(window);
    		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) printf("W key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) printf("S key was pressed!\n");

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) printf("A key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) printf("D key was pressed!\n");

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) printf("LSHIFT key was pressed!\n");
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) printf("SPACE key was pressed!\n");

		usleep(10000);
	}

	glfwTerminate();
}

















	// printf("printing physical devices available:\n");
	// for (uint32_t i = 0; i < physical_device_count; i++) {

	// 	printf("physical device #%d : %p\n", i, (void*) physical_devices[i]);

	// 	VkPhysicalDeviceProperties properties = {0};
	// 	vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
		
	// 	printf("\tname : %s       api=%d . driver=%d . v_id=%d . d_id=%d \n", 
	// 		properties.deviceName, 
	// 		properties.apiVersion,
	// 		properties.driverVersion,
	// 		properties.vendorID,	
	// 		properties.deviceID
	// 	);
	// }
	// printf(".\n");











	// uint32_t device_ext_count = 10;
	// VkExtensionProperties device_exts[10] = {0};

	// vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_ext_count, device_exts);




// VkDebugReportCallbackEXT debug_report;
	// if (vkCreateDebugReportCallbackEXT(instance, &debug_report_info, NULL, &debug_report) != VK_SUCCESS) 
	// 	vk_error("create debug report call back");

	// printf("debug: set call back.\n");


