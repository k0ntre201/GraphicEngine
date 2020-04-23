#pragma once

#define GLFW_INCLUDE_VULKAN

#include "../GlfwWindow.hpp"

namespace GraphicEngine::GLFW
{
	class GlfwVulkanWindow : public GlfwWindow
	{
	public:
		GlfwVulkanWindow();
	};
}