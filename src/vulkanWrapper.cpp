#include "../include/VulkanWrapper.h"



#include <iostream>
#include <set>
#include <string>
#include <stdexcept>
#include <cstdlib>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);

}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void VulkanWrapper::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool VulkanWrapper::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

bool VulkanWrapper::isDeviceSuitable(VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> props(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, props.data());
	int index, i = 0;
	// find a device with a graphics queue family && presentation caps
	for (auto p : props)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if ((p.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && presentSupport)
		{
			graphics_family_queue_index = i;
			break;
		}
		i++;
	}
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	return graphics_family_queue_index.has_value() && extensionsSupported;
}

bool VulkanWrapper::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool VulkanWrapper::selectPhysicalDevice()
{
	std::vector<VkPhysicalDevice> availableVulkanDevices;
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);
	if (0 == count)
	{
		std::cerr << "No GPU with Vulkan support availabe!\n";
		return false;
	}
	availableVulkanDevices.resize(count);

	vkEnumeratePhysicalDevices(instance, &count, availableVulkanDevices.data());
	for (auto device : availableVulkanDevices)
	{
		if (isDeviceSuitable(device))
		{
			physical_device = device;
			break;
		}
	}
	if (physical_device == nullptr)
	{
		std::cerr << "No Suitable GPU was found!\n";
		return false;
	}
	return true;
}

bool VulkanWrapper::createLogicalDevice()
{
	VkDeviceCreateInfo info{};
	VkDeviceQueueCreateInfo qinfo{};
	float priority = 1.0;
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.queueCreateInfoCount = 1;
	info.pQueueCreateInfos = &qinfo;
	info.enabledExtensionCount = deviceExtensions.size();
	info.ppEnabledExtensionNames = deviceExtensions.data();

	qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qinfo.queueCount = 1;
	qinfo.queueFamilyIndex = graphics_family_queue_index.value();
	qinfo.pQueuePriorities = &priority;

	if (VK_SUCCESS != vkCreateDevice(physical_device, &info, nullptr, &device))
	{
		std::cerr << "Failed to create logical device!\n";
		return false;
	}

	vkGetDeviceQueue(device, graphics_family_queue_index.value(), 0, &graphics_queue);

	return true;

}

void VulkanWrapper::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

bool VulkanWrapper::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		std::cerr << "failed to create window surface!\n";
		return false;
	}
	return true;
}
void VulkanWrapper::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
}

void VulkanWrapper::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VulkanWrapper::cleanup()
{
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanWrapper::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(win_width, win_height, "Learning Vulkan!", nullptr, nullptr);
}

bool VulkanWrapper::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		std::cerr << "validation layers requested, but not available!\n";
		return false;
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Learning vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "no Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	auto extensions = getRequiredExtensions();
	info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	info.ppEnabledExtensionNames = extensions.data();

	info.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		info.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		info.enabledLayerCount = 0;
		info.pNext = nullptr;
	}

	if (VK_SUCCESS != vkCreateInstance(&info, nullptr, &instance))
	{
		std::cerr << "Failed to create Vulkan instance\n";
		return false;
	}
	return true;
}

std::vector<const char*> VulkanWrapper::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}