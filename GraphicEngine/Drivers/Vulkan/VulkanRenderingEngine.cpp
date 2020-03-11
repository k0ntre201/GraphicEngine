#include "VulkanRenderingEngine.hpp"
#include "../../Platform/Glew/WindowGLFW.hpp"

#undef max

GraphicEngine::Vulkan::VulkanRenderingEngine::VulkanRenderingEngine(std::shared_ptr<Window> window) :
	RenderingEngine(window)
{
}

bool GraphicEngine::Vulkan::VulkanRenderingEngine::drawFrame()
{
	_device->waitForFences(1, &(_renderingBarriers->inFlightFences[currentFrameIndex].get()), true, std::numeric_limits<uint64_t>::max());
	vk::ResultValue<uint32_t> imageIndex = _device->acquireNextImageKHR(_swapChainData.swapChain.get(), std::numeric_limits<uint64_t>::max(), _renderingBarriers->imageAvailableSemaphores[currentFrameIndex].get(), vk::Fence());

	if (imageIndex.result == vk::Result::eErrorOutOfDateKHR)
	{
		// Should recreate swapchain
		return false;
	}

	else if (imageIndex.result != vk::Result::eSuccess && imageIndex.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("Failed to acquire next image!");
	}

	if (_renderingBarriers->imagesInFlight[imageIndex.value].get() != vk::Fence())
		_device->waitForFences(1, &(_renderingBarriers->inFlightFences[imageIndex.value].get()), true, std::numeric_limits<uint64_t>::max());

	_renderingBarriers->imagesInFlight[imageIndex.value].get() = _renderingBarriers->inFlightFences[currentFrameIndex].get();

	_device->resetFences(1, &(_renderingBarriers->inFlightFences[currentFrameIndex].get()));

	vk::Semaphore waitSemaphore(_renderingBarriers->imageAvailableSemaphores[currentFrameIndex].get());
	vk::Semaphore signalSemaphore(_renderingBarriers->renderFinishedSemaphores[currentFrameIndex].get());
	vk::PipelineStageFlags pipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::SubmitInfo submitInfo(1, &waitSemaphore, &pipelineStageFlags, 0, &(_commandBuffers[imageIndex.value].get()), 1, &signalSemaphore);

	vk::Result submitResult = _graphicQueue.submit(1, &submitInfo, _renderingBarriers->inFlightFences[currentFrameIndex].get());
	if (submitResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	vk::SwapchainKHR sp(_swapChainData.swapChain.get());

	vk::PresentInfoKHR presentInfo(1, &signalSemaphore, 1, &sp, &imageIndex.value);
	vk::Result presentResult = _presentQueue.presentKHR(presentInfo);

	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || frameBufferResized)
	{
		frameBufferResized = false;
		// Should recreate swapchain
	}

	currentFrameIndex = calculateNextIndex();
	return true;
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::init(size_t width, size_t height)
{
	try
	{
		_instance = createUniqueInstance("Graphic Engine", "Vulklan Base", validationLayers, _window->getRequiredExtensions(), VK_API_VERSION_1_0);
		{
			auto surface = std::dynamic_pointer_cast<GraphicEngine::GLFW::WindowGLFW>(_window)->getWindowSurface(_instance);
			vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(_instance.get());
			_surface = vk::UniqueSurfaceKHR(surface, _deleter);
		}
		_physicalDevice = getPhysicalDevice(_instance, _surface);
		_device = getUniqueLogicalDevice(_physicalDevice, _surface);
		indices = findGraphicAndPresentQueueFamilyIndices(_physicalDevice, _surface);

		auto [w, h] = _window->getFrameBufferSize();
		vk::Extent2D frameBufferSize(w, h);
		_swapChainData = SwapChainData(_physicalDevice, _device, _surface, indices, frameBufferSize, vk::UniqueSwapchainKHR(), vk::ImageUsageFlagBits::eColorAttachment);

		maxFrames = _swapChainData.images.size();

		_commandPool = createUniqueCommandPool(_device, indices);
		_commandBuffers = _device->allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo(_commandPool.get(), vk::CommandBufferLevel::ePrimary, maxFrames));

		_depthBuffer = std::unique_ptr<DeepBufferData>(new DeepBufferData(_physicalDevice, _device, vk::Extent3D(frameBufferSize, 1), findDepthFormat(_physicalDevice), msaaSamples));
		_rendePass = createRenderPass(_device, _swapChainData.format, _depthBuffer->format, msaaSamples);
		_image = std::unique_ptr<ImageData>(new ImageData(_physicalDevice, _device,
			vk::Extent3D(_swapChainData.extent, 1), _swapChainData.format, msaaSamples,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
			vk::ImageTiling::eOptimal, 1, vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eColor));
		_frameBuffers = createFrameBuffers(_device, _rendePass, _swapChainData.extent, 1, _image->imageView, _depthBuffer->imageView, _swapChainData.imageViews);

		_graphicQueue = _device->getQueue(indices.graphicsFamily.value(), 0);
		_presentQueue = _device->getQueue(indices.presentFamily.value(), 0);

		_renderingBarriers = std::unique_ptr<RenderingBarriers>(new RenderingBarriers(_device, maxFrames));

		buildCommandBuffers();
	}

	catch (vk::SystemError & err)
	{
		throw std::runtime_error(err.what());
	}
	catch (...)
	{
		throw std::runtime_error("Vulkan Initialize: unknown error\n");
	}
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::resizeFrameBuffer(size_t width, size_t height)
{
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::cleanup()
{
	_device->waitIdle();
}

uint32_t GraphicEngine::Vulkan::VulkanRenderingEngine::calculateNextIndex()
{
	return (currentFrameIndex + 1) % maxFrames;
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::buildCommandBuffers()
{
	std::array<vk::ClearValue, 3> clearValues;

	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.2f,0.2f,0.2f,1.0f }));
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0.0f);
	clearValues[2].color = vk::ClearColorValue(std::array<float, 4>({ 0.2f,0.2f,0.2f,1.0f }));
	int i{ 0 };
	for (auto& commandBuffer : _commandBuffers)
	{
		commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));
		vk::RenderPassBeginInfo renderPassBeginInfo(_rendePass.get(), _frameBuffers[i].get(), vk::Rect2D(vk::Offset2D(0, 0), _swapChainData.extent), static_cast<uint32_t>(clearValues.size()), clearValues.data());
		commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer->setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapChainData.extent.width), static_cast<float>(_swapChainData.extent.height), 0.0f, 1.0f));
		commandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapChainData.extent));
		commandBuffer->endRenderPass();
		commandBuffer->end();
		++i;
	}
}
