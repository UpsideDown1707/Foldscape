#pragma once

#include "physicaldevice.hpp"

namespace foldscape::vk
{
	class Vulkan;

	class ShaderModule
	{
		const Vulkan& m_vulkan;
		VkShaderModule m_shaderModule;

	public:
		ShaderModule(const Vulkan& vulkan, const std::vector<char>& code);
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule(ShaderModule&& rhs);
		~ShaderModule();
		inline VkShaderModule* operator&() { return &m_shaderModule; }
		inline operator VkShaderModule() const { return m_shaderModule; }
	};
	class VulkanResources
	{
		VulkanResources(const VulkanResources&) = delete;
		VulkanResources(VulkanResources&&) = delete;
		void operator=(const VulkanResources&) = delete;
		void operator=(VulkanResources&&) = delete;

	protected:
		//VkAllocationCallbacks m_allocator;
		VkInstance m_instance;
#if VALIDATION_LAYER_ENABLED
		VkDebugUtilsMessengerEXT m_debugMessenger;
#endif
		VkDevice m_device;
		VkCommandPool m_commandPool;
		VkCommandBuffer m_commandBuffer;

	protected:
		VulkanResources();
		~VulkanResources();
		inline VkAllocationCallbacks* Allocator() const { return nullptr; }
	};

	class Vulkan : private VulkanResources
	{
		VkQueue m_queue;
		PhysicalDevice m_physicalDevice;

	private:
		void CreateInstance(const char* name);
		PhysicalDevice SelectPhysicalDevice() const;
		void CreateLogicalDevice();
		void CreateCommandPool();
		void CreateCmdBuffer();

	public:
		Vulkan(const char* name);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		inline const PhysicalDevice& Gpu() const { return m_physicalDevice; }
		inline VkDevice Device() const { return m_device; }
		inline VkCommandPool CommandPool() const { return m_commandPool; }
		inline VkQueue Queue() const { return m_queue; }
		inline VkCommandBuffer CommandBuffer() const { return m_commandBuffer; }
		inline VkAllocationCallbacks* Allocator() const { return VulkanResources::Allocator(); }
	};
}
