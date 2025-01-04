#pragma once

#include "shaderimage.hpp"
#include "panandzoom2d.hpp"
#include <complex>

namespace foldscape
{
	class MandelImage : public ShaderImage, public PanAndZoom2D
	{
		struct alignas(16) ShaderParameters
		{
			PanAndZoomParams pzParams;
			uint32_t maxIters;
		};

	
	private:
		vk::UniformBuffer m_parameterBuffer;

	private:
		void CreateDescriptorSet();
	
	public:
		MandelImage(vk::Vulkan& vulkan, IDrawContext& drawContext, int width, int height);
		virtual void Resize(int width, int height) override;
		virtual void Render() override;
	};
}