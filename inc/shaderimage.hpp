#pragma once

#include "vk/buffer.hpp"

namespace foldscape
{
	class ShaderImageResources
	{
	protected:
		vk::Vulkan& m_vulkan;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
		VkFence m_fence;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

	protected:
		explicit ShaderImageResources(vk::Vulkan& vulkan);
		~ShaderImageResources();
		void ClearDescriptorResources();
	};

	class ShaderImage : protected ShaderImageResources
	{
	protected:
		vk::TexelBuffer m_image;
	
	protected:
		void CreatePipeline(const char* shaderFilename, const VkDescriptorSetLayoutBinding bindings[], uint32_t bindingCount);

	public:
		ShaderImage(vk::Vulkan& vulkan, int width, int height);
	};
}