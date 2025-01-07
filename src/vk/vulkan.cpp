#include "vk/singletimecommandbuffer.hpp"
#include "common.hpp"
#include <iostream>
#include <algorithm>

namespace foldscape::vk
{
#if VALIDATION_LAYER_ENABLED
	static const char* const g_ValidationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	static bool CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : g_ValidationLayers)
		{
			bool layerFound = false;

			for (const VkLayerProperties& layerProperties : availableLayers)
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData)
	{

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	static VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator)
	{
	    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	    if (func != nullptr)
	        func(instance, debugMessenger, pAllocator);
	}
#endif

	VulkanResources::VulkanResources()
		: m_instance{VK_NULL_HANDLE}
#if VALIDATION_LAYER_ENABLED
		, m_debugMessenger{VK_NULL_HANDLE}
#endif
		, m_device{VK_NULL_HANDLE}
		, m_commandPool{VK_NULL_HANDLE}
		, m_commandBuffer{VK_NULL_HANDLE}
	{}

	VulkanResources::~VulkanResources()
	{
		SAFE_DESTROY(vkFreeCommandBuffers, m_commandBuffer, m_device, m_commandPool, 1, &m_commandBuffer);
		SAFE_DESTROY(vkDestroyCommandPool, m_commandPool, m_device, m_commandPool, Allocator());

		SAFE_DESTROY(vkDestroyDevice, m_device, m_device, Allocator());
#if VALIDATION_LAYER_ENABLED
		SAFE_DESTROY(DestroyDebugUtilsMessengerEXT, m_debugMessenger, m_instance, m_debugMessenger, Allocator());
#endif
		SAFE_DESTROY(vkDestroyInstance, m_instance, m_instance, Allocator());
	}

	ShaderModule::ShaderModule(const Vulkan& vulkan, const std::vector<char>& code)
		: m_vulkan{vulkan}
		, m_shaderModule{VK_NULL_HANDLE}
	{
		VkShaderModuleCreateInfo moduleInfo{};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleInfo.codeSize = code.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		ThrowIfFailed(vkCreateShaderModule(m_vulkan.Device(), &moduleInfo, m_vulkan.Allocator(), &m_shaderModule));
	}

	ShaderModule::ShaderModule(ShaderModule&& rhs)
		: m_vulkan{rhs.m_vulkan}
		, m_shaderModule{rhs.m_shaderModule}
	{
		rhs.m_shaderModule = VK_NULL_HANDLE;
	}

	ShaderModule::~ShaderModule()
	{
		SAFE_DESTROY(vkDestroyShaderModule, m_shaderModule, m_vulkan.Device(), m_shaderModule, m_vulkan.Allocator());
	}

	void Vulkan::CreateInstance(const char* name)
	{
#if VALIDATION_LAYER_ENABLED
		ThrowIfFalse(CheckValidationLayerSupport());
#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = name;
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;

#if VALIDATION_LAYER_ENABLED
		const char* extensions[] = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};
		instanceInfo.enabledExtensionCount = ARRAY_SIZE(extensions);
		instanceInfo.ppEnabledExtensionNames = extensions;
		instanceInfo.enabledLayerCount = ARRAY_SIZE(g_ValidationLayers);
		instanceInfo.ppEnabledLayerNames = g_ValidationLayers;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = DebugCallback;
		debugCreateInfo.pUserData = static_cast<void*>(this);
		instanceInfo.pNext = static_cast<void*>(&debugCreateInfo);
#endif

		ThrowIfFailed(vkCreateInstance(&instanceInfo, Allocator(), &m_instance));

#if VALIDATION_LAYER_ENABLED
		ThrowIfFailed(CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, Allocator(), &m_debugMessenger));
#endif
	}

	PhysicalDevice Vulkan::SelectPhysicalDevice() const
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		ThrowIfFalse(deviceCount);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		std::vector<PhysicalDevice> devData;
		devData.reserve(deviceCount);
		uint32_t bestDeviceIndex = 0;
		uint32_t bestDeviceScore = 0;

		for (uint32_t i = 0; i < deviceCount; ++i)
		{
			devData.emplace_back(devices[i]);
			const uint32_t score = devData[i].ScoreDevice();
			if (bestDeviceScore < score)
			{
				bestDeviceScore = score;
				bestDeviceIndex = i;
			}
		}
		ThrowIfFalse(bestDeviceScore);

		return devData[bestDeviceIndex];
	}

	void Vulkan::CreateLogicalDevice()
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};

		float queuePriority = 1.0f;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_physicalDevice.QueueIndex();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.shaderFloat64 = VK_TRUE;

		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceInfo.pEnabledFeatures = &deviceFeatures;
		deviceInfo.enabledExtensionCount = PhysicalDevice::sDeviceExtensions.size();
		deviceInfo.ppEnabledExtensionNames = PhysicalDevice::sDeviceExtensions.data();
#if VALIDATION_LAYER_ENABLED
		deviceInfo.enabledLayerCount = ARRAY_SIZE(g_ValidationLayers);
		deviceInfo.ppEnabledLayerNames = g_ValidationLayers;
#endif
		ThrowIfFailed(vkCreateDevice(m_physicalDevice.Device(), &deviceInfo, Allocator(), &m_device));
		vkGetDeviceQueue(m_device, m_physicalDevice.QueueIndex(), 0, &m_queue);
	}

	void Vulkan::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_physicalDevice.QueueIndex();
		ThrowIfFailed(vkCreateCommandPool(m_device, &poolInfo, Allocator(), &m_commandPool));
	}

	void Vulkan::CreateCmdBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer));
	}

	Vulkan::Vulkan(const char* name)
		: m_queue{VK_NULL_HANDLE}
		, m_physicalDevice{}
	{
		CreateInstance(name);
		m_physicalDevice = SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateCmdBuffer();
	}

	uint32_t Vulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice.Device(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		Throw("failed to find suitable memory type");
	}
}
