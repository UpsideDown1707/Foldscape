#include "nebulaimage.hpp"

namespace foldscape
{
	void NebulaImage::CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[3]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		poolSizes[2].descriptorCount = 1;

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

		VkWriteDescriptorSet descriptorWrites[3]{};

		VkDescriptorBufferInfo storageBufferInfo{};
		storageBufferInfo.offset = 0;
		storageBufferInfo.range = m_countsBuffer.Size();
		storageBufferInfo.buffer = m_countsBuffer.Buffer();
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &storageBufferInfo;
		descriptorWrites[0].dstSet = m_descriptorSet;

		VkDescriptorBufferInfo uniformBufferInfo{};
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = sizeof(ShaderParameters);
		uniformBufferInfo.buffer = m_parameterBuffer.Buffer();
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &uniformBufferInfo;
		descriptorWrites[1].dstSet = m_descriptorSet;

		VkBufferView view = m_image.View();
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pTexelBufferView = &view;
		descriptorWrites[2].dstSet = m_descriptorSet;

		vkUpdateDescriptorSets(m_vulkan.Device(), ARRAY_SIZE(descriptorWrites), descriptorWrites, 0, nullptr);
	}

	NebulaImage::NebulaImage(vk::Vulkan& vulkan, IDrawContext& drawContext, int width, int height)
		: ShaderImage(vulkan, width, height)
		, PanAndZoom2D(drawContext, width, height)
		, m_countsPipeline(vulkan)
		, m_colorPipeline(vulkan)
		, m_countsBuffer(vulkan, width * height * 3 * sizeof(uint32_t))
		, m_parameterBuffer(vulkan, sizeof(ShaderParameters))
	{
		m_pzParams.center = {-0.5, 0.0};
		m_pzParams.zoom = 1.25;

		VkDescriptorSetLayoutBinding bindings[3]{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		bindings[2].descriptorCount = 1;
		bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		CreatePipelineLayout(bindings, ARRAY_SIZE(bindings));
		m_countsPipeline.Init(m_pipelineLayout, "nebula_counts_comp.spv");
		m_colorPipeline.Init(m_pipelineLayout, "nebula_color_comp.spv");
		CreateDescriptorSet();
		CreateSurface(m_image.MappedData(), width, height);
	}

	void NebulaImage::Resize(int width, int height)
	{
		SafeDestroySurface();
		const bool imgResized = m_image.Resize(width, height);
		const bool countsBufferResized = m_countsBuffer.Resize(width * height * 3 * sizeof(uint32_t));
		if (imgResized || countsBufferResized)
		{
			ClearDescriptorResources();
			CreateDescriptorSet();
		}
		CreateSurface(m_image.MappedData(), width, height);
		m_pzParams.resolution = {width, height};
	}

	void NebulaImage::Render()
	{
		const VkCommandBuffer cmdBuffer = m_vulkan.CommandBuffer();

		vkResetFences(m_vulkan.Device(), 1, &m_fence);
		vkResetCommandBuffer(cmdBuffer, 0);

		const float pixelMultiplier = 3.0f;
		const uint32_t maxIters[3] = { 5000, 1600, 120 };
		ShaderParameters* params = m_parameterBuffer.MappedData<ShaderParameters*>();
		params->center = std::complex<float>(float(m_pzParams.center.real()), float(m_pzParams.center.imag()));
		params->zoom = float(m_pzParams.zoom);
		params->resolution = m_pzParams.resolution;
		params->maxIters[0] = maxIters[0];
		params->maxIters[1] = maxIters[1];
		params->maxIters[2] = maxIters[2];
		params->colorMultipliers[0] = 0.3f / (pixelMultiplier * pixelMultiplier * std::log(float(maxIters[0])));
		params->colorMultipliers[1] = 0.3f / (pixelMultiplier * pixelMultiplier * std::log(float(maxIters[1])));
		params->colorMultipliers[2] = 0.3f / (pixelMultiplier * pixelMultiplier * std::log(float(maxIters[2])));
		params->firstDrawnPointIter = 5;
		params->pixelMultiplier = pixelMultiplier;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		ThrowIfFailed(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_countsPipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdDispatch(cmdBuffer, (m_image.Width() + 7) / 8, (m_image.Height() + 7) / 8, 1);

		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_colorPipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdDispatch(cmdBuffer, (m_image.Width() * m_image.Height() + 63) / 64, 1, 1);

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