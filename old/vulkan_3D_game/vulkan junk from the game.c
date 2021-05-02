

// LOGI("Initializing vulkan instance.");

// 	if (volkInitialize())
// 	{
// 		throw std::runtime_error("Failed to initialize volk.");
// 	}

// 	uint32_t instance_extension_count;
// 	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

// 	std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);
// 	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

// 	std::vector<const char *> active_instance_extensions(required_instance_extensions);

// #if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
// 	active_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
// #endif

// #if defined(VK_USE_PLATFORM_ANDROID_KHR)
// 	active_instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
// #elif defined(VK_USE_PLATFORM_WIN32_KHR)
// 	active_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
// #elif defined(VK_USE_PLATFORM_MACOS_MVK)
// 	active_instance_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
// #elif defined(VK_USE_PLATFORM_XCB_KHR)
// 	active_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
// #elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
// 	active_instance_extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
// #else
// #	pragma error Platform not supported
// #endif

	// if (!validate_extensions(active_instance_extensions, instance_extensions))
	// {
	// 	throw std::runtime_error("Required instance extensions are missing.");
	// }

	// uint32_t instance_layer_count;
	// VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

	// std::vector<VkLayerProperties> supported_validation_layers(instance_layer_count);
	// VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, supported_validation_layers.data()));

	// std::vector<const char *> requested_validation_layers(required_validation_layers);

// #ifdef VKB_VALIDATION_LAYERS
// 	// Determine the optimal validation layers to enable that are necessary for useful debugging
// 	std::vector<const char *> optimal_validation_layers = vkb::get_optimal_validation_layers(supported_validation_layers);
// 	requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
// #endif

	// if (validate_layers(requested_validation_layers, supported_validation_layers))
	// {
	// 	LOGI("Enabled Validation Layers:")
	// 	for (const auto &layer : requested_validation_layers)
	// 	{
	// 		LOGI("	\t{}", layer);
	// 	}
	// }
	// else
	// {
	// 	throw std::runtime_error("Required validation layers are missing.");
	// }

	// VkApplicationInfo app = {0};
	// app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	// app.pApplicationName = "Hello Triangle";
	// app.pEngineName      = "Vulkan Samples";
	// app.apiVersion       = VK_MAKE_VERSION(1, 0, 0);

	// VkInstanceCreateInfo instance_info = {0};
	// instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	// instance_info.pApplicationInfo        = &app;
	// instance_info.enabledExtensionCount   = vkb::to_u32(active_instance_extensions.size());
	// instance_info.ppEnabledExtensionNames = active_instance_extensions.data();
	// instance_info.enabledLayerCount       = vkb::to_u32(requested_validation_layers.size());
	// instance_info.ppEnabledLayerNames     = requested_validation_layers.data();

// #if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
// 	VkDebugReportCallbackCreateInfoEXT debug_report_create_info = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
// 	debug_report_create_info.flags                              = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
// 	debug_report_create_info.pfnCallback                        = debug_callback;

// 	instance_info.pNext = &debug_report_create_info;
// #endif

	// Create the Vulkan instance
	// vkCreateInstance(&instance_info, nullptr, &context.instance);


// #if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
// 	VK_CHECK(vkCreateDebugReportCallbackEXT(context.instance, &debug_report_create_info, nullptr, &context.debug_callback));
// #endif












	// uint32_t extension_count;
	// const char** required_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
	// printf("required extensions: \n");
	// for (uint32_t i = 0; i < extension_count; i++) {
	// 	printf("\t - %s\n", required_extensions[i]);
	// }
	// printf(".\n");


	// const char** extensions = malloc(sizeof(const char*) * (extension_count + 1));

	// for (uint32_t i = 0; i < extension_count; i++) {
	// 	extensions[i] = required_extensions[i];
	// }
	// extensions




