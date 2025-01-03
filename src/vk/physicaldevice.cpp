#include "vk/physicaldevice.hpp"
#include <iostream>

namespace foldscape::vk
{
	constexpr uint32_t ACCEPTABLE_SCORE = 1;
	constexpr uint32_t DISCETE_GPU_SCORE = 20;
	constexpr uint32_t INTEGRATED_GPU_SCORE = 10;
	constexpr uint32_t VIRTUAL_GPU_SCORE = 5;

	const std::vector<const char*> PhysicalDevice::sDeviceExtensions = {
	};

	bool PhysicalDevice::CheckDeviceExtensionSupport() const
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, availableExtensions.data());

		for (const char* devExtension : PhysicalDevice::sDeviceExtensions)
		{
			bool extensionFound = false;
			for (const VkExtensionProperties& ext : availableExtensions)
			{
				if (0 == strcmp(devExtension, ext.extensionName))
				{
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound)
				return false;
		}

		return true;
	}

	uint32_t PhysicalDevice::ScorePhysicalDeviceQueues()
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, queueFamilies.data());

		m_queueIndex = UINT32_MAX;

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				m_queueIndex = i;
				return ACCEPTABLE_SCORE;
			}
		}

		return 0;
	}

	uint32_t PhysicalDevice::ScoreGpuType()
	{
		switch (m_properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return DISCETE_GPU_SCORE;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return INTEGRATED_GPU_SCORE;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return VIRTUAL_GPU_SCORE;
		default:
			return ACCEPTABLE_SCORE;
		}
	}

	PhysicalDevice::PhysicalDevice(VkPhysicalDevice device)
		: m_device{device}
		, m_queueIndex{}
	{}

	uint32_t PhysicalDevice::ScoreDevice()
	{
		vkGetPhysicalDeviceProperties(m_device, &m_properties);
		vkGetPhysicalDeviceFeatures(m_device, &m_features);

		if (!CheckDeviceExtensionSupport())
			return 0;

		const uint32_t queueScore = ScorePhysicalDeviceQueues();
		if (!queueScore)
			return 0;

		const uint32_t gpuTypeScore = ScoreGpuType();

		return queueScore + gpuTypeScore;
	}
}
