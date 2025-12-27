#pragma once
#include <vulkan/vulkan_core.h>

namespace Rendering { class Camera; }

namespace Rendering
{
	struct FrameInfo
	{
		int frameIndex;
		float frame_dt;
		VkCommandBuffer commandBuffer;
		Camera &rCamera;
		VkDescriptorSet globalDescSet;
	};
}