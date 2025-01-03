#pragma once

#include "physicaldevice.hpp"

namespace foldscape::vk
{
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
		VkBuffer m_image;
		VkBufferView m_view;
		VkDeviceMemory m_imageMemory;
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_bufferMemory;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
		VkFence m_fence;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

	protected:
		VulkanResources();
		~VulkanResources();
		inline VkAllocationCallbacks* Allocator() const { return nullptr; }
	};

	class Vulkan : private VulkanResources
	{
		class ShaderModule
		{
			const Vulkan& m_gfx;
			VkShaderModule m_shaderModule;

		public:
			ShaderModule(const Vulkan& gfx);
			ShaderModule(const ShaderModule&) = delete;
			ShaderModule(ShaderModule&& rhs);
			~ShaderModule();
			inline VkShaderModule* operator&() { return &m_shaderModule; }
			inline operator VkShaderModule() const { return m_shaderModule; }
		};

		struct ShaderParams
		{
			int width;
			int height;
		};

	private:
		VkQueue m_queue;
		PhysicalDevice m_physicalDevice;
		int m_width;
		int m_height;
		void* m_mappedImage;
		ShaderParams* m_mappedBuffer;

	private:
		void CreateInstance(const char* name);
		PhysicalDevice SelectPhysicalDevice() const;
		void CreateLogicalDevice();
		ShaderModule CreateShaderModule(const std::vector<char>& code) const;
		void CreateCommandPool();
		void CreateSyncObjects();
		void CreateImage();
		void CreateUniformBuffer();
		void CreatePipeline();
		void CreateCmdBuffer();
		void CreateDescriptorSet();

		void RecordCommandBuffer();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	public:
		Vulkan(const char* name, int width, int height);
		void Resize(int width, int height);
		bool Render();

		inline void Flush() const { vkDeviceWaitIdle(m_device); }

		inline const PhysicalDevice& Gpu() const { return m_physicalDevice; }
		inline VkDevice Device() const { return m_device; }
		inline VkCommandPool CommandPool() const { return m_commandPool; }
		inline VkQueue Queue() const { return m_queue; }
		inline VkCommandBuffer CommandBuffer() const { return m_commandBuffer; }
		inline VkAllocationCallbacks* Allocator() const { return VulkanResources::Allocator(); }
		inline void* MappedImage() const { return m_mappedImage; }
	};
}
