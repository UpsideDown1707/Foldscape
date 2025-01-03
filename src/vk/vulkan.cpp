#include "vk/singletimecommandbuffer.hpp"
#include "common.hpp"
#include <iostream>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
		, m_image{VK_NULL_HANDLE}
		, m_view{VK_NULL_HANDLE}
		, m_imageMemory{VK_NULL_HANDLE}
		, m_uniformBuffer{VK_NULL_HANDLE}
		, m_bufferMemory{VK_NULL_HANDLE}
		, m_pipelineLayout{VK_NULL_HANDLE}
		, m_pipeline{VK_NULL_HANDLE}
		, m_fence{VK_NULL_HANDLE}
		, m_descriptorSetLayout{VK_NULL_HANDLE}
		, m_descriptorPool{VK_NULL_HANDLE}
		, m_descriptorSet{VK_NULL_HANDLE}
	{}

	VulkanResources::~VulkanResources()
	{
		SAFE_DESTROY(vkDestroyPipeline, m_pipeline, m_device, m_pipeline, Allocator());
		SAFE_DESTROY(vkDestroyPipelineLayout, m_pipelineLayout, m_device, m_pipelineLayout, Allocator());
		if (m_descriptorSet)
		{
			vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &m_descriptorSet);
			m_descriptorSet = VK_NULL_HANDLE;
		}
		SAFE_DESTROY(vkDestroyDescriptorPool, m_descriptorPool, m_device, m_descriptorPool, Allocator());
		SAFE_DESTROY(vkDestroyDescriptorSetLayout, m_descriptorSetLayout, m_device, m_descriptorSetLayout, Allocator());

		SAFE_DESTROY(vkDestroyBufferView, m_view, m_device, m_view, Allocator());
		SAFE_DESTROY(vkDestroyBuffer, m_image, m_device, m_image, Allocator());
		SAFE_DESTROY(vkFreeMemory, m_imageMemory, m_device, m_imageMemory, Allocator());
		SAFE_DESTROY(vkDestroyBuffer, m_uniformBuffer, m_device, m_uniformBuffer, Allocator());
		SAFE_DESTROY(vkFreeMemory, m_bufferMemory, m_device, m_bufferMemory, Allocator());

		if (m_commandBuffer)
		{
			vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
			m_commandBuffer = VK_NULL_HANDLE;
		}
		SAFE_DESTROY(vkDestroyFence, m_fence, m_device, m_fence, Allocator());
		SAFE_DESTROY(vkDestroyCommandPool, m_commandPool, m_device, m_commandPool, Allocator());

		SAFE_DESTROY(vkDestroyDevice, m_device, m_device, Allocator());
#if VALIDATION_LAYER_ENABLED
		SAFE_DESTROY(DestroyDebugUtilsMessengerEXT, m_debugMessenger, m_instance, m_debugMessenger, Allocator());
#endif
		SAFE_DESTROY(vkDestroyInstance, m_instance, m_instance, Allocator());
	}

	Vulkan::ShaderModule::ShaderModule(const Vulkan& gfx)
		: m_gfx{gfx}
		, m_shaderModule{VK_NULL_HANDLE}{}

	Vulkan::ShaderModule::ShaderModule(ShaderModule&& rhs)
		: m_gfx{rhs.m_gfx}
		, m_shaderModule{rhs.m_shaderModule}
	{
		rhs.m_shaderModule = VK_NULL_HANDLE;
	}

	Vulkan::ShaderModule::~ShaderModule()
	{
		SAFE_DESTROY(vkDestroyShaderModule, m_shaderModule, m_gfx.m_device, m_shaderModule, m_gfx.Allocator());
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

	Vulkan::ShaderModule Vulkan::CreateShaderModule(const std::vector<char>& code) const
	{
		VkShaderModuleCreateInfo moduleInfo{};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleInfo.codeSize = code.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        ShaderModule shaderModule(*this);
		ThrowIfFailed(vkCreateShaderModule(m_device, &moduleInfo, Allocator(), &shaderModule));
		return shaderModule;
	}

	void Vulkan::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_physicalDevice.QueueIndex();
		ThrowIfFailed(vkCreateCommandPool(m_device, &poolInfo, Allocator(), &m_commandPool));
	}

	void Vulkan::CreateSyncObjects()
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		ThrowIfFailed(vkCreateFence(m_device, &fenceInfo, Allocator(), &m_fence));
	}
	
	void Vulkan::CreateImage()
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = m_width * m_height * 4;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ThrowIfFailed(vkCreateBuffer(m_device, &bufferInfo, Allocator(), &m_image));

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(m_device, m_image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		ThrowIfFailed(vkAllocateMemory(m_device, &allocInfo, Allocator(), &m_imageMemory));
		ThrowIfFailed(vkBindBufferMemory(m_device, m_image, m_imageMemory, 0));

		vkMapMemory(m_device, m_imageMemory, 0, VK_WHOLE_SIZE, 0, &m_mappedImage);

		VkBufferViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		viewInfo.buffer = m_image;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.range = VK_WHOLE_SIZE;
		ThrowIfFailed(vkCreateBufferView(m_device, &viewInfo, Allocator(), &m_view));
	}

	void Vulkan::CreateUniformBuffer()
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(ShaderParams);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ThrowIfFailed(vkCreateBuffer(m_device, &bufferInfo, Allocator(), &m_uniformBuffer));

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(m_device, m_uniformBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		ThrowIfFailed(vkAllocateMemory(m_device, &allocInfo, Allocator(), &m_bufferMemory));

		ThrowIfFailed(vkBindBufferMemory(m_device, m_uniformBuffer, m_bufferMemory, 0));
		ThrowIfFailed(vkMapMemory(m_device, m_bufferMemory, 0, memRequirements.size, 0, reinterpret_cast<void**>(&m_mappedBuffer)));
	}

	void Vulkan::CreatePipeline()
	{
        const std::vector<char> shaderCode = ReadFile((GetProgramFolder() + "shader_comp.spv").c_str());
		ShaderModule shaderModule = CreateShaderModule(shaderCode);
		
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = shaderModule;
		stageInfo.pName = "main";

		VkDescriptorSetLayoutBinding bindings[2]{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = ARRAY_SIZE(bindings);
		layoutInfo.pBindings = bindings;
		ThrowIfFailed(vkCreateDescriptorSetLayout(m_device, &layoutInfo, Allocator(), &m_descriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		ThrowIfFailed(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, Allocator(), &m_pipelineLayout));
		
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.stage = stageInfo;
		ThrowIfFailed(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, Allocator(), &m_pipeline));
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

	void Vulkan::CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		ThrowIfFailed(vkCreateDescriptorPool(m_device, &poolInfo, Allocator(), &m_descriptorPool));

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout;
		ThrowIfFailed(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet));

		VkWriteDescriptorSet descriptorWrites[2]{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pTexelBufferView = &m_view;
		descriptorWrites[0].dstSet = m_descriptorSet;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ShaderParams);
		bufferInfo.buffer = m_uniformBuffer;
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo;
		descriptorWrites[1].dstSet = m_descriptorSet;
		vkUpdateDescriptorSets(m_device, ARRAY_SIZE(descriptorWrites), descriptorWrites, 0, nullptr);
	}

	void Vulkan::RecordCommandBuffer()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		ThrowIfFailed(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
		vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdDispatch(m_commandBuffer, (m_width + 15) / 16, (m_height + 15) / 16, 1);
		ThrowIfFailed(vkEndCommandBuffer(m_commandBuffer));
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

	Vulkan::Vulkan(const char* name, int width, int height)
		: m_queue{VK_NULL_HANDLE}
		, m_physicalDevice{}
		, m_width{width}
		, m_height{height}
		, m_mappedImage{}
		, m_mappedBuffer{}
	{
		CreateInstance(name);
		m_physicalDevice = SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateCmdBuffer();
		CreateSyncObjects();
		CreateImage();
		CreateUniformBuffer();
		CreatePipeline();
		CreateDescriptorSet();
	}

	void Vulkan::Resize(int width, int height)
	{
		if (width > 0 && height > 0 && (width != m_width || height != m_height))
		{
			m_width = width;
			m_height = height;

			SAFE_DESTROY(vkFreeDescriptorSets, m_descriptorSet, m_device, m_descriptorPool, 1, &m_descriptorSet);
			SAFE_DESTROY(vkDestroyDescriptorPool, m_descriptorPool, m_device, m_descriptorPool, Allocator());
			SAFE_DESTROY(vkDestroyBufferView, m_view, m_device, m_view, Allocator());
			SAFE_DESTROY(vkDestroyBuffer, m_image, m_device, m_image, Allocator());
			SAFE_DESTROY(vkFreeMemory, m_imageMemory, m_device, m_imageMemory, Allocator());
			m_mappedImage = nullptr;

			CreateImage();
			CreateDescriptorSet();
		}
	}

	bool Vulkan::Render()
	{
		m_mappedBuffer->height = m_height;
		m_mappedBuffer->width = m_width;
		vkResetFences(m_device, 1, &m_fence);
		vkResetCommandBuffer(m_commandBuffer, 0);

		RecordCommandBuffer();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, m_fence));
		vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);

		return true;
	}
}
