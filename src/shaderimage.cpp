#include "shaderimage.hpp"

namespace foldscape
{
	ShaderImageResources::ShaderImageResources(vk::Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_pipelineLayout{VK_NULL_HANDLE}
		, m_fence{VK_NULL_HANDLE}
		, m_descriptorSetLayout{VK_NULL_HANDLE}
		, m_descriptorPool{VK_NULL_HANDLE}
		, m_descriptorSet{VK_NULL_HANDLE}
	{}

	ShaderImageResources::~ShaderImageResources()
	{
		ClearDescriptorResources();
		SAFE_DESTROY(vkDestroyPipelineLayout, m_pipelineLayout, m_vulkan.Device(), m_pipelineLayout, m_vulkan.Allocator());
		SAFE_DESTROY(vkDestroyDescriptorSetLayout, m_descriptorSetLayout, m_vulkan.Device(), m_descriptorSetLayout, m_vulkan.Allocator());

		SAFE_DESTROY(vkDestroyFence, m_fence, m_vulkan.Device(), m_fence, m_vulkan.Allocator());
	}
	
	void ShaderImageResources::ClearDescriptorResources()
	{
		SAFE_DESTROY(vkFreeDescriptorSets, m_descriptorSet, m_vulkan.Device(), m_descriptorPool, 1, &m_descriptorSet);
		SAFE_DESTROY(vkDestroyDescriptorPool, m_descriptorPool, m_vulkan.Device(), m_descriptorPool, m_vulkan.Allocator());
	}

	void ShaderImage::CreatePipelineLayout(const VkDescriptorSetLayoutBinding bindings[], uint32_t bindingCount)
	{
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