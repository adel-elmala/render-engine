#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

class VulkanWrapper {
public:
	void run();

private:
	void initVulkan();
	void mainLoop();
	void cleanup();
	void initWindow();
	bool createInstance();
	bool createSurface();
	bool checkValidationLayerSupport();
	void setupDebugMessenger();
	bool selectPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool createLogicalDevice();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device;
	VkQueue graphics_queue;
	VkDevice device;
	std::optional<uint32_t> graphics_family_queue_index;
	GLFWwindow* window;
	unsigned int win_width = 800;
	unsigned int win_height = 600;

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

};
