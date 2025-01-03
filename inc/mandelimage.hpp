#pragma once

#include "shaderimage.hpp"

namespace foldscape
{
	class MandelImage : public ShaderImage
	{
	public:
		struct Parameters
		{
			int width;
			int height;
		};
	
	private:
		vk::UniformBuffer m_parameters;

	private:
		void CreateDescriptorSet();

	public:
		MandelImage(vk::Vulkan& vulkan, int width, int height);
		virtual void Resize(int width, int height) override;
		virtual void Render() override;
	};
}