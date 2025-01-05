#include "vk/computepipeline.hpp"

namespace foldscape::vk
{
	ComputePipeline::ComputePipeline(const Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_pipeline{VK_NULL_HANDLE}
	{}

	ComputePipeline::ComputePipeline(const Vulkan& vulkan, VkPipelineLayout layout, const char* shaderFileName)
		: ComputePipeline(vulkan)
	{
		Init(layout, shaderFileName);
	}

	ComputePipeline::~ComputePipeline()
	{
		SAFE_DESTROY(vkDestroyPipeline, m_pipeline, m_vulkan.Device(), m_pipeline, m_vulkan.Allocator());
	}

	void ComputePipeline::Init(VkPipelineLayout layout, const char* shaderFileName)
	{
		const std::vector<char> shaderCode = ReadFile((GetProgramFolder() + shaderFileName).c_str());
		vk::ShaderModule shaderModule(m_vulkan, shaderCode);
		
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = shaderModule;
		stageInfo.pName = "main";
		
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = layout;
		pipelineInfo.stage = stageInfo;
		ThrowIfFailed(vkCreateComputePipelines(m_vulkan.Device(), VK_NULL_HANDLE, 1, &pipelineInfo, m_vulkan.Allocator(), &m_pipeline));
	}
}
