#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <iso646.h>
#include <stdnoreturn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define window_width 1200
#define window_height 800
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



#define get(name) PFN_##name name = (PFN_##name) glfwGetInstanceProcAddress(NULL, #name)

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


static inline void* open_file(const char* filename, size_t* out_length) {
	struct stat file_data = {0};
	int file = open(filename, O_RDONLY);
	if (file < 0 or stat(filename, &file_data) < 0) { perror("open"); exit(3); }
	size_t length = (size_t) file_data.st_size;
	void* input = not length ? 0 : mmap(0, length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);
	*out_length = length;
	return input;
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
	get(vkGetPhysicalDeviceQueueFamilyProperties);
	get(vkCreateDevice);
	get(vkCreateShaderModule);
	get(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	get(vkGetPhysicalDeviceSurfaceFormatsKHR);
	get(vkGetPhysicalDeviceSurfaceSupportKHR);
	get(vkCreateSwapchainKHR);
	get(vkGetSwapchainImagesKHR);
	get(vkCreateRenderPass);

	const char* instance_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME, 
		"VK_EXT_metal_surface", 
		"VK_EXT_validation_features", 
		"VK_KHR_get_physical_device_properties2"
	};
	const uint32_t instance_extension_count = 4;

	const char* device_extensions[] = {
		"VK_KHR_portability_subset", 
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const uint32_t device_extension_count = 2;

	const char* validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
	const uint32_t validation_layer_count = 1;


	VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
	uint32_t enabled_validation_feature_count = 1;

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
 		.enabledValidationFeatureCount = enabled_validation_feature_count,
		.pEnabledValidationFeatures = enables
	};

	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &features,
		.pApplicationInfo = &application,
		.enabledExtensionCount = instance_extension_count, 
		.ppEnabledExtensionNames = instance_extensions,
		.enabledLayerCount = validation_layer_count,
		.ppEnabledLayerNames = validation_layers
	};

	printf("debug: creating instance...\n");
	VkInstance instance;
	if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS) 
		vk_error("create instance");


	printf("debug: creating window surface...\n");
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (err) fail("window surface");


	printf("debug: enumerating the physical devices...\n");
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);
	VkPhysicalDevice* physical_devices = calloc(physical_device_count, sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);
	printf("debug: ---> found %d physical devices.\n", physical_device_count);


	if (not physical_device_count) vk_error("no suitable GPU physical device found");
	VkPhysicalDevice physical_device = physical_devices[0];


	printf("debug: getting the queues...\n");
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties* queue_family_properties = calloc(queue_family_count,  sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);
	printf("debug: ---> got %d queues.\n", queue_family_count);


	printf("debug: finding queue family index...\n");
	uint32_t queue_index = 0;
	for (; queue_index < queue_family_count; queue_index++) {
		if (glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_index)) 
			break;
	}

	if (queue_index == queue_family_count) 
		vk_error("no suitable queue family index found for the device");
	
	printf("debug: ---> using queue_family_index = %d    (out of %d)\n", queue_index, queue_family_count);


	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_index, surface, &supported);
	if (supported == VK_FALSE) abort();
	

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
		.enabledExtensionCount = device_extension_count,
		.ppEnabledExtensionNames = device_extensions,
	};

	printf("debug: creating the device...\n");
	VkDevice device;
	if (vkCreateDevice(physical_device, &device_info, NULL, &device) != VK_SUCCESS) 
		vk_error("create device");

	printf("debug: getting the queue...\n");
	VkQueue queue;
	vkGetDeviceQueue(device, queue_index, 0, &queue);


	printf("debug: getting physical device surface capabilities...\n");
	VkSurfaceCapabilitiesKHR surface_properties;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_properties) != VK_SUCCESS)
		vk_error("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");


	printf("debug: getting physical device surface formats...\n");
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
	VkSurfaceFormatKHR* formats = calloc(format_count,  sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);

	printf("debug: ---> got %d surface formats.\n", format_count);

	printf("debug: printing formats:\n");
	for (uint32_t i = 0; i < format_count; i++) {
		printf("\t- format #%d : %d\n", i, formats[i].format);
	}
	printf(".\n");

	printf("debug: picking a format....\n");
	VkSurfaceFormatKHR format;
	format.format = VK_FORMAT_UNDEFINED;

	for (uint32_t i = 0; i < format_count; i++) {
		if (	formats[i].format == VK_FORMAT_R8G8B8A8_UNORM or 
			formats[i].format == VK_FORMAT_B8G8R8A8_UNORM or
			formats[i].format == VK_FORMAT_A8B8G8R8_UNORM_PACK32
		) { format = formats[i]; break; }
	}

	if (format.format == VK_FORMAT_UNDEFINED) {
		format = formats[0];
	}
	printf("debug: ---> picked format: %d.\n", format.format);
	
	VkExtent2D swapchain_size = surface_properties.currentExtent;

	printf("debug: ---> found swap_chain size: [width=%d, height=%d]\n", swapchain_size.width, swapchain_size.height);
	
	VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR; // or mailbox for triple buffering?...

	uint32_t desired_swapchain_images = surface_properties.minImageCount + 1;
	if ((surface_properties.maxImageCount > 0) && (desired_swapchain_images > surface_properties.maxImageCount))
	{
		desired_swapchain_images = surface_properties.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR pre_transform;
	if (surface_properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		pre_transform = surface_properties.currentTransform;
	}


	VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}

	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = desired_swapchain_images,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent.width = swapchain_size.width,
		.imageExtent.height = swapchain_size.height,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = pre_transform,
		.compositeAlpha = composite,
		.presentMode = swapchain_present_mode,
		.clipped = true,
		.oldSwapchain = NULL
	};

	printf("debug: creating swapchain...\n");

	VkSwapchainKHR swapchain;
	if (vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain) != VK_SUCCESS) 
		vk_error("create swapchain");
	

	printf("debug: getting swapchain images...\n");
	uint32_t image_count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);
	VkImage* swapchain_images = calloc(image_count,  sizeof(VkImage));
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images);
	printf("debug: got %d images.\n", image_count);














	// for (size_t i = 0; i < image_count; i++) {
	// 	context.per_frame[i]

	// 	VkFenceCreateInfo info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	// 	info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	// 	VK_CHECK(vkCreateFence(context.device, &info, nullptr, &per_frame.queue_submit_fence));

	// 	VkCommandPoolCreateInfo cmd_pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	// 	cmd_pool_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	// 	cmd_pool_info.queueFamilyIndex = context.graphics_queue_index;
	// 	VK_CHECK(vkCreateCommandPool(context.device, &cmd_pool_info, nullptr, &per_frame.primary_command_pool));

	// 	VkCommandBufferAllocateInfo cmd_buf_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	// 	cmd_buf_info.commandPool        = per_frame.primary_command_pool;
	// 	cmd_buf_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	// 	cmd_buf_info.commandBufferCount = 1;
	// 	VK_CHECK(vkAllocateCommandBuffers(context.device, &cmd_buf_info, &per_frame.primary_command_buffer));

	// 	per_frame.device      = context.device;
	// 	per_frame.queue_index = context.graphics_queue_index;
	// }









	// for (size_t i = 0; i < image_count; i++)
	// {
	// 	// Create an image view which we can render into.
	// 	VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	// 	view_info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
	// 	view_info.format                      = context.swapchain_dimensions.format;
	// 	view_info.image                       = swapchain_images[i];
	// 	view_info.subresourceRange.levelCount = 1;
	// 	view_info.subresourceRange.layerCount = 1;
	// 	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 	view_info.components.r                = VK_COMPONENT_SWIZZLE_R;
	// 	view_info.components.g                = VK_COMPONENT_SWIZZLE_G;
	// 	view_info.components.b                = VK_COMPONENT_SWIZZLE_B;
	// 	view_info.components.a                = VK_COMPONENT_SWIZZLE_A;

	// 	VkImageView image_view;
	// 	VK_CHECK(vkCreateImageView(context.device, &view_info, nullptr, &image_view));

	// 	context.swapchain_image_views.push_back(image_view);
	// }































	//--------------  load shader modules: ----------------
	
	size_t code_length = 0;
	printf("debug: reading in shader vertex spir-v file...\n");
	unsigned int* code = open_file("shaders/shader.vert.spv", &code_length);

	VkShaderModuleCreateInfo vertex_module_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code_length,
		.pCode    = code,
	};

	printf("debug: creating vertex shader module from %lu bytes...\n", code_length);
	VkShaderModule vertex_shader_module;
	if (vkCreateShaderModule(device, &vertex_module_info, NULL, &vertex_shader_module) != VK_SUCCESS)
		vk_error("create vertex shader module");
	
	munmap(code, code_length);
	


	printf("debug: reading in shader fragment spir-v file...\n");
	code = open_file("shaders/shader.frag.spv", &code_length);

	VkShaderModuleCreateInfo fragment_module_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code_length,
		.pCode    = code,
	};

	printf("debug: creating fragment shader module from %lu bytes...\n", code_length);
	VkShaderModule fragment_shader_module;
	if (vkCreateShaderModule(device, &fragment_module_info, NULL, &fragment_shader_module) != VK_SUCCESS)
		vk_error("create fragment shader module");
	
	munmap(code, code_length);
	





























	VkAttachmentDescription attachment = {
		.format = format.format,
		.samples = VK_SAMPLE_COUNT_1_BIT, 
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, 
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	VkSubpassDescription subpass = {
		.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments    = &color_ref,
	};

	VkSubpassDependency dependency = {
		.srcSubpass          = VK_SUBPASS_EXTERNAL,
		.dstSubpass          = 0,

		.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount        = 1,
		.pAttachments           = &attachment,
		.subpassCount           = 1,
		.pSubpasses             = &subpass,
		.dependencyCount        = 1,
		.pDependencies          = &dependency,
	};


	printf("debug: creating render pass...\n");

	VkRenderPass render_pass;
	if (vkCreateRenderPass(device, &rp_info, NULL, &render_pass) != VK_SUCCESS) 
		vk_error("create render pass");
	




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


// uint32_t property_count = 100;
	// VkExtensionProperties properties[100] = {0};
	// if (vkEnumerateDeviceExtensionProperties(physical_device, NULL, &property_count, properties) != VK_SUCCESS) 
	// 	vk_error("enumerate device extension properties");
















	// uint32_t image_count = swapChainSupport.capabilities.minImageCount + 1;

 //        VkSwapchainCreateInfoKHR createInfo = {
 //        	.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
 //        	.surface = surface,
 //        	.minImageCount = image_count,
 //        	.imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
 //        	.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
 //        	.imageExtent = extent,
 //        	.imageArrayLayers = 1,
 //        	.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	// 	.preTransform = swapChainSupport.capabilities.currentTransform,
 //        	.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
 //        	.presentMode = VK_PRESENT_MODE_MAILBOX_KHR, // VK_PRESENT_MODE_FIFO_KHR, if not supported.
 //        	.clipped = VK_TRUE,
 //        	.oldSwapchain = NULL,
	// 	.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	// };

        
	// if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) 
	// 	vk_error("create swapchain");
        

	// VkImage images[100] = {0};
	// vkGetSwapchainImagesKHR(device, swapChain, &image_count, images);
       

 //        vkimageformat swapChainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
 //        extent;







