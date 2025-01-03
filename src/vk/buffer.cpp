#include "vk/buffer.hpp"

namespace foldscape::vk
{
	UniformBufferResources::UniformBufferResources(const Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_buffer{VK_NULL_HANDLE}
		, m_memory{VK_NULL_HANDLE}
	{}

	UniformBufferResources::~UniformBufferResources()
	{
		SAFE_DESTROY(vkDestroyBuffer, m_buffer, m_vulkan.Device(), m_buffer, m_vulkan.Allocator());
		SAFE_DESTROY(vkFreeMemory, m_memory, m_vulkan.Device(), m_memory, m_vulkan.Allocator());
	}

	UniformBuffer::UniformBuffer(const Vulkan& vulkan, VkDeviceSize size)
		: UniformBufferResources(vulkan)
		, m_size{size}
		, m_mappedData{nullptr}
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ThrowIfFailed(vkCreateBuffer(m_vulkan.Device(), &bufferInfo, m_vulkan.Allocator(), &m_buffer));

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(m_vulkan.Device(), m_buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_vulkan.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		ThrowIfFailed(vkAllocateMemory(m_vulkan.Device(), &allocInfo, m_vulkan.Allocator(), &m_memory));

		ThrowIfFailed(vkBindBufferMemory(m_vulkan.Device(), m_buffer, m_memory, 0));
		ThrowIfFailed(vkMapMemory(m_vulkan.Device(), m_memory, 0, VK_WHOLE_SIZE, 0, &m_mappedData));
	}

	TexelBufferResources::TexelBufferResources(const Vulkan& vulkan)
		: m_vulkan{vulkan}
		, m_buffer{VK_NULL_HANDLE}
		, m_memory{VK_NULL_HANDLE}
		, m_view{VK_NULL_HANDLE}
	{}

	TexelBufferResources::~TexelBufferResources()
	{
		ClearResources();
	}

	void TexelBufferResources::ClearResources()
	{
		SAFE_DESTROY(vkDestroyBufferView, m_view, m_vulkan.Device(), m_view, m_vulkan.Allocator());
		SAFE_DESTROY(vkDestroyBuffer, m_buffer, m_vulkan.Device(), m_buffer, m_vulkan.Allocator());
		SAFE_DESTROY(vkFreeMemory, m_memory, m_vulkan.Device(), m_memory, m_vulkan.Allocator());
	}

	void TexelBuffer::CreateImage()
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = m_width * m_height * 4;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ThrowIfFailed(vkCreateBuffer(m_vulkan.Device(), &bufferInfo, m_vulkan.Allocator(), &m_buffer));

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(m_vulkan.Device(), m_buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_vulkan.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		ThrowIfFailed(vkAllocateMemory(m_vulkan.Device(), &allocInfo, m_vulkan.Allocator(), &m_memory));
		ThrowIfFailed(vkBindBufferMemory(m_vulkan.Device(), m_buffer, m_memory, 0));

		vkMapMemory(m_vulkan.Device(), m_memory, 0, VK_WHOLE_SIZE, 0, &m_mappedData);

		VkBufferViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		viewInfo.buffer = m_buffer;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.range = VK_WHOLE_SIZE;
		ThrowIfFailed(vkCreateBufferView(m_vulkan.Device(), &viewInfo, m_vulkan.Allocator(), &m_view));
	}

	TexelBuffer::TexelBuffer(const Vulkan& vulkan, int width, int height)
		: TexelBufferResources(vulkan)
		, m_width{width}
		, m_height{height}
		, m_mappedData{nullptr}
	{
		CreateImage();
	}

	bool TexelBuffer::Resize(int width, int height)
	{
		if (width > 0 && height > 0 && (width != m_width || height != m_height))
		{
			ClearResources();
			m_width = width;	
			m_height = height;
			m_mappedData = nullptr;
			CreateImage();
			return true;
		}
		return false;
	}
}