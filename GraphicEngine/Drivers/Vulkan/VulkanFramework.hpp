#pragma once

#include "VulkanHelper.hpp"
#include "VulkanWindowContext.hpp"

namespace GraphicEngine::Vulkan
{
	class VulkanFramework
	{
	public:
		VulkanFramework() = default;
		VulkanFramework(std::shared_ptr<VulkanWindowContext> vulkanWindowsContext, const std::string& appName, 
			const std::string& engineName, int width, int height, vk::SampleCountFlagBits msaaSamples,
			const std::vector<std::string>& validationLayers);

		VulkanFramework& initialize(std::shared_ptr<VulkanWindowContext> vulkanWindowsContext, const std::string& appName,
			const std::string& engineName, int width, int height, vk::SampleCountFlagBits msaaSamples,
			const std::vector<std::string>& validationLayers);

		VulkanFramework& initializeFramebuffer(int width, int height);
		VulkanFramework& initializeFramebuffer();

		VulkanFramework& initializeCommandBuffer();
		
		VulkanFramework& initalizeRenderingBarriers();

		template <typename T>
		std::unique_ptr<UniformBuffer<T>> getUniformBuffer();

		virtual ~VulkanFramework() = default;
	protected:
	private:
		std::shared_ptr<VulkanWindowContext> m_vulkanWindowContext;

		vk::UniqueInstance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueSurfaceKHR m_surface;
		vk::UniqueDevice m_device;
		vk::Queue m_graphicQueue;
		vk::Queue m_presentQueue;

		SwapChainData m_swapChainData;
		std::unique_ptr<DepthBufferData> m_depthBuffer;
		vk::UniqueRenderPass m_renderPass;
		std::unique_ptr<ImageData> m_image;
		std::vector<vk::UniqueFramebuffer> m_frameBuffers;

		vk::UniqueCommandPool m_commandPool;
		std::vector<vk::UniqueCommandBuffer> m_commandBuffers;

		std::unique_ptr<RenderingBarriers> m_renderingBarriers;

	private:
		std::string m_appName;
		std::string m_engineName;
		int m_width;
		int m_height;

	private:
		vk::SampleCountFlagBits m_msaaSamples;
		uint32_t m_maxFrames{ 1 };
		uint32_t m_currentFrameIndex{ 0 };
		QueueFamilyIndices m_indices;

		std::shared_ptr<VulkanWindowContext> m_vulkanWindowContext;
	private:
		std::vector<std::string> m_validationLayers;
	};
}