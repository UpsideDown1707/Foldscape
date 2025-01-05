#pragma once

#include "vk/vulkan.hpp"

namespace foldscape::vk
{
	class ComputePipeline
	{
		const Vulkan& m_vulkan;
		VkPipeline m_pipeline;
	
	public:
		ComputePipeline(const Vulkan& vulkan);
		ComputePipeline(const Vulkan& vulkan, VkPipelineLayout layout, const char* shaderFileName);
		~ComputePipeline();

		void Init(VkPipelineLayout layout, const char* shaderFileName);

		inline operator VkPipeline() { return m_pipeline; }
	};
}