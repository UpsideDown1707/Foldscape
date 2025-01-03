#include "shaderimage.hpp"

namespace foldscape
{
	ShaderImageResources::ShaderImageResources(vk::Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_pipelineLayout{VK_NULL_HANDLE}
		, m_pipeline{VK_NULL_HANDLE}
		, m_fence{VK_NULL_HANDLE}
		, m_descriptorSetLayout{VK_NULL_HANDLE}
		, m_descriptorPool{VK_NULL_HANDLE}
		, m_descriptorSet{VK_NULL_HANDLE}
	{}

	ShaderImageResources::~ShaderImageResources()
	{
		ClearDescriptorResources();
		SAFE_DESTROY(vkDestroyPipeline, m_pipeline, m_vulkan.Device(), m_pipeline, m_vulkan.Allocator());
		SAFE_DESTROY(vkDestroyPipelineLayout, m_pipelineLayout, m_vulkan.Device(), m_pipelineLayout, m_vulkan.Allocator());
		SAFE_DESTROY(vkDestroyDescriptorSetLayout, m_descriptorSetLayout, m_vulkan.Device(), m_descriptorSetLayout, m_vulkan.Allocator());

		SAFE_DESTROY(vkDestroyFence, m_fence, m_vulkan.Device(), m_fence, m_vulkan.Allocator());
	}
	
	void ShaderImageResources::ClearDescriptorResources()
	{
		SAFE_DESTROY(vkFreeDescriptorSets, m_descriptorSet, m_vulkan.Device(), m_descriptorPool, 1, &m_descriptorSet);
		SAFE_DESTROY(vkDestroyDescriptorPool, m_descriptorPool, m_vulkan.Device(), m_descriptorPool, m_vulkan.Allocator());
	}

	void ShaderImage::CreatePipeline(const char* shaderFilename, const VkDescriptorSetLayoutBinding bindings[], uint32_t bindingCount)
	{
		const std::vector<char> shaderCode = ReadFile((GetProgramFolder() + "shader_comp.spv").c_str());
		vk::ShaderModule shaderModule(m_vulkan, shaderCode);
		
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = shaderModule;
		stageInfo.pName = "main";

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindingCount;
		layoutInfo.pBindings = bindings;
		ThrowIfFailed(vkCreateDescriptorSetLayout(m_vulkan.Device(), &layoutInfo, m_vulkan.Allocator(), &m_descriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		ThrowIfFailed(vkCreatePipelineLayout(m_vulkan.Device(), &pipelineLayoutInfo, m_vulkan.Allocator(), &m_pipelineLayout));
		
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.stage = stageInfo;
		ThrowIfFailed(vkCreateComputePipelines(m_vulkan.Device(), VK_NULL_HANDLE, 1, &pipelineInfo, m_vulkan.Allocator(), &m_pipeline));
	}

	ShaderImage::ShaderImage(vk::Vulkan& vulkan, int width, int height)
		: ShaderImageResources(vulkan)
		, m_image(vulkan, width, height)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		ThrowIfFailed(vkCreateFence(m_vulkan.Device(), &fenceInfo, m_vulkan.Allocator(), &m_fence));
	}
}