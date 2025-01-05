#pragma once

#include "vk/computepipeline.hpp"
#include "shaderimage.hpp"
#include "panandzoom2d.hpp"
#include <complex>

namespace foldscape
{
	class NebulaImage : public ShaderImage, public PanAndZoom2D
	{
		struct ShaderParameters
		{
			std::complex<float> center;
			float zoom;
			uint8_t padding1[4];
			int2 resolution;
			uint8_t padding2[8];
			uint32_t maxIters[3];
			uint8_t padding3[4];
			float colorMultipliers[3];
			uint32_t firstDrawnPointIter;
			float pixelMultiplier;
		};
	
	private:
		vk::ComputePipeline m_countsPipeline;
		vk::ComputePipeline m_colorPipeline;
		vk::StorageBuffer m_countsBuffer;
		vk::UniformBuffer m_parameterBuffer;

	private:
		void CreateDescriptorSet();
	
	public:
		NebulaImage(vk::Vulkan& vulkan, IDrawContext& drawContext, int width, int height);
		virtual void Resize(int width, int height) override;
		virtual void Render() override;
	};
}