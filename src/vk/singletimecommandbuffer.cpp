#include "vk/singletimecommandbuffer.hpp"

namespace foldscape::vk
{
	SingleTimeCommandBufferResources::SingleTimeCommandBufferResources(const Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_commandBuffer{VK_NULL_HANDLE}
	{}

	SingleTimeCommandBufferResources::~SingleTimeCommandBufferResources()
	{
		if (m_commandBuffer)
		{
			vkFreeCommandBuffers(m_vulkan.Device(), m_vulkan.CommandPool(), 1, &m_commandBuffer);
			m_commandBuffer = nullptr;
		}
	}

	SingleTimeCommandBuffer::SingleTimeCommandBuffer(const Vulkan& vulkan)
		: SingleTimeCommandBufferResources{vulkan}
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_vulkan.CommandPool();
		allocInfo.commandBufferCount = 1;
		ThrowIfFailed(vkAllocateCommandBuffers(m_vulkan.Device(), &allocInfo, &m_commandBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		ThrowIfFailed(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
	}

	void SingleTimeCommandBuffer::Submit()
	{
		vkEndCommandBuffer(m_commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffer;
		ThrowIfFailed(vkQueueSubmit(m_vulkan.Queue(), 1, &submitInfo, VK_NULL_HANDLE));

		ThrowIfFailed(vkQueueWaitIdle(m_vulkan.Queue()));
		vkFreeCommandBuffers(m_vulkan.Device(), m_vulkan.CommandPool(), 1, &m_commandBuffer);
		m_commandBuffer = nullptr;
	}
}
