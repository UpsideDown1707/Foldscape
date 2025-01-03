#pragma once

#include "common.hpp"

namespace foldscape::vk
{
	class PhysicalDevice
	{
	private:
		VkPhysicalDevice m_device;
		uint32_t m_queueIndex;
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;

	public:
		static const std::vector<const char*> sDeviceExtensions;

	private:
		bool CheckDeviceExtensionSupport() const;
		uint32_t ScorePhysicalDeviceQueues();
		uint32_t ScoreGpuType();

	public:
		PhysicalDevice(VkPhysicalDevice device = VK_NULL_HANDLE);
		uint32_t ScoreDevice();

		inline VkPhysicalDevice Device() const { return m_device; }
		inline uint32_t QueueIndex() const { return m_queueIndex; }
		inline const VkPhysicalDeviceProperties& Properties() const { return m_properties; }
		inline const VkPhysicalDeviceFeatures& Features() const { return m_features; }
	};
}
