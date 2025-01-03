#pragma once

#include <vk/vulkan.hpp>

namespace foldscape::vk
{
	class SingleTimeCommandBufferResources
	{
		SingleTimeCommandBufferResources(const SingleTimeCommandBufferResources&) = delete;
		SingleTimeCommandBufferResources(SingleTimeCommandBufferResources&&) = delete;
		void operator=(const SingleTimeCommandBufferResources&) = delete;
		void operator=(SingleTimeCommandBufferResources&&) = delete;
	protected:
		const Vulkan& m_vulkan;
		VkCommandBuffer m_commandBuffer;

	protected:
		explicit SingleTimeCommandBufferResources(const Vulkan& vulkan);
		~SingleTimeCommandBufferResources();
	};

	class SingleTimeCommandBuffer : private SingleTimeCommandBufferResources
	{
	public:
		explicit SingleTimeCommandBuffer(const Vulkan& vulkan);
		inline VkCommandBuffer CommandBuffer() const { return m_commandBuffer; }
		void Submit();
	};
}
