#include "mandelimage.hpp"

namespace foldscape
{
	void MandelImage::CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		ThrowIfFailed(vkCreateDescriptorPool(m_vulkan.Device(), &poolInfo, m_vulkan.Allocator(), &m_descriptorPool));

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout;
		ThrowIfFailed(vkAllocateDescriptorSets(m_vulkan.Device(), &allocInfo, &m_descriptorSet));

		VkWriteDescriptorSet descriptorWrites[2]{};

		VkBufferView view = m_image.View();
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pTexelBufferView = &view;
		descriptorWrites[0].dstSet = m_descriptorSet;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Parameters);
		bufferInfo.buffer = m_parameters.Buffer();
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo;
		descriptorWrites[1].dstSet = m_descriptorSet;

		vkUpdateDescriptorSets(m_vulkan.Device(), ARRAY_SIZE(descriptorWrites), descriptorWrites, 0, nullptr);
	}

	MandelImage::MandelImage(vk::Vulkan& vulkan, int width, int height)
		: ShaderImage(vulkan, width, height)
		, m_parameters(vulkan, sizeof(Parameters))
	{
		VkDescriptorSetLayoutBinding bindings[2]{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		CreatePipeline("shader_comp.spv", bindings, ARRAY_SIZE(bindings));
		CreateDescriptorSet();
	}

	void MandelImage::Resize(int width, int height)
	{
		if (m_image.Resize(width, height))
		{
			ClearDescriptorResources();
			CreateDescriptorSet();
		}
	}

	void MandelImage::Render()
	{
		const VkCommandBuffer cmdBuffer = m_vulkan.CommandBuffer();

		vkResetFences(m_vulkan.Device(), 1, &m_fence);
		vkResetCommandBuffer(cmdBuffer, 0);

		Parameters* params = m_parameters.MappedData<Parameters*>();
		params->width = m_image.Width();
		params->height = m_image.Height();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		ThrowIfFailed(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdDispatch(cmdBuffer, (m_image.Width() + 15) / 16, (m_image.Height() + 15) / 16, 1);
		ThrowIfFailed(vkEndCommandBuffer(m_vulkan.CommandBuffer()));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		ThrowIfFailed(vkQueueSubmit(m_vulkan.Queue(), 1, &submitInfo, m_fence));
		vkWaitForFences(m_vulkan.Device(), 1, &m_fence, VK_TRUE, UINT64_MAX);
	}
}