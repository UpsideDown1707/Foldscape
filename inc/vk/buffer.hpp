#pragma once

#include "vulkan.hpp"

namespace foldscape::vk
{
	class UniformBufferResources
	{
	protected:
		const Vulkan& m_vulkan;
		VkBuffer m_buffer;
		VkDeviceMemory m_memory;
	
	protected:
		explicit UniformBufferResources(const Vulkan& vulkan);
		~UniformBufferResources();
	};

	class UniformBuffer : private UniformBufferResources
	{
		VkDeviceSize m_size;
		void* m_mappedData;

	public:
		UniformBuffer(const Vulkan& vulkan, VkDeviceSize size);

		inline VkBuffer Buffer() const { return m_buffer; }
		inline VkDeviceSize Size() const { return m_size; }
		template <typename T = void*>
		inline T MappedData() const { return reinterpret_cast<T>(m_mappedData); }
	};

	class TexelBufferResources
	{
	protected:
		const Vulkan& m_vulkan;
		VkBuffer m_buffer;
		VkDeviceMemory m_memory;
		VkBufferView m_view;
	
	protected:
		explicit TexelBufferResources(const Vulkan& vulkan);
		~TexelBufferResources();
		void ClearResources();
	};

	class TexelBuffer : private TexelBufferResources
	{
		int m_width;
		int m_height;
		void* m_mappedData;
	
	private:
		void CreateImage();

	public:
		TexelBuffer(const Vulkan& vulkan, int width, int height);

		bool Resize(int width, int height);
		
		inline VkBufferView View() const { return m_view; }
		inline void* MappedData() const { return m_mappedData; }
		inline int Width() const { return m_width; }
		inline int Height() const { return m_height; }
	};
}