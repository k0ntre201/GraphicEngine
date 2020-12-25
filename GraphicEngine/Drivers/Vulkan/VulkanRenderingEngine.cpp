#include "VulkanRenderingEngine.hpp"

#include "../../Core/IO/FileReader.hpp"
#include "../../Core/IO/FileSystem.hpp"

#include "VulkanTextureFactory.hpp"
#include "VulkanVertexBufferFactory.hpp"

#undef max

GraphicEngine::Vulkan::VulkanRenderingEngine::VulkanRenderingEngine(std::shared_ptr<VulkanWindowContext> vulkanWindowContext,
	std::shared_ptr<Services::CameraControllerManager> cameraControllerManager,
	std::shared_ptr<Services::ModelManager> modelManager,
	std::shared_ptr<Core::EventManager> eventManager,
	std::shared_ptr<Common::UI> ui,
	std::shared_ptr<Core::Configuration> cfg,
	std::unique_ptr<Core::Logger<VulkanRenderingEngine>> logger) :
	m_vulkanWindowContext(vulkanWindowContext),
	RenderingEngine(cameraControllerManager, modelManager, eventManager, ui, cfg)
{
}

bool GraphicEngine::Vulkan::VulkanRenderingEngine::drawFrame()
{
	try
	{
		m_framework->acquireFrame();
		buildCommandBuffers();

		auto eyePosition = m_cameraControllerManager->getActiveCamera()->getPosition();
		Engines::Graphic::Shaders::Light light{ eyePosition, glm::vec3{ 1.0f } };
		Engines::Graphic::Shaders::Eye eye{ eyePosition };
		auto cameraMatrix = m_cameraControllerManager->getActiveCamera()->getViewProjectionMatrix();
		m_uniformBuffer->updateAndSet(cameraMatrix);
		m_eyePositionUniformBuffer->updateAndSet(eye);
		m_lightUniformBuffer->updateAndSet(light);

		m_wireframeGraphicPipeline->updateDynamicUniforms();
		m_solidColorraphicPipeline->updateDynamicUniforms();
		m_normalDebugGraphicPipeline->updateDynamicUniforms();

		m_framework->submitFrame();
	}

	catch (vk::OutOfDateKHRError err)
	{
		return false;
	}

	return true;
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::init(size_t width, size_t height)
{
	try
	{
		m_framework = std::make_shared<VulkanFramework>();
		m_framework->
			initialize(m_vulkanWindowContext, "Graphic Engine", "Vulkan Base", width, height, vk::SampleCountFlagBits::e2, { "VK_LAYER_KHRONOS_validation" }, std::make_unique<Core::Logger<VulkanFramework>>())
			.initializeCommandBuffer()
			.initializeFramebuffer()
			.initalizeRenderingBarriers();

		m_uniformBuffer = m_framework->getUniformBuffer<UniformBuffer, glm::mat4>();
		m_lightUniformBuffer = m_framework->getUniformBuffer<UniformBuffer, Engines::Graphic::Shaders::Light>();
		m_eyePositionUniformBuffer = m_framework->getUniformBuffer<UniformBuffer, Engines::Graphic::Shaders::Eye>();

		m_wireframeGraphicPipeline = std::make_shared<VulkanWireframeGraphicPipeline>(m_framework, m_uniformBuffer);
		m_solidColorraphicPipeline = std::make_shared<VulkanSolidColorGraphicPipeline>(m_framework, m_uniformBuffer, m_lightUniformBuffer, m_eyePositionUniformBuffer, m_cameraControllerManager);
		m_normalDebugGraphicPipeline = std::make_shared<VulkanNormalDebugGraphicPipeline>(m_framework, m_uniformBuffer, m_cameraControllerManager);

		m_modelManager->getModelEntityContainer()->forEachEntity([&](auto model)
		{
			for (auto mesh : model->getMeshes())
			{
				auto vb = mesh->compile<VertexBufferFactory, VertexBuffer>(m_framework->m_physicalDevice, m_framework->m_device, m_framework->m_commandPool, m_framework->m_graphicQueue);
				m_wireframeGraphicPipeline->addVertexBuffer<decltype(mesh)::element_type::vertex_type>(mesh, vb);
				m_solidColorraphicPipeline->addVertexBuffer<decltype(mesh)::element_type::vertex_type>(mesh, vb);
				m_normalDebugGraphicPipeline->addVertexBuffer<decltype(mesh)::element_type::vertex_type>(mesh, vb);
			}
		});
		m_uiRenderingBackend = std::make_shared<GUI::ImGuiImpl::VulkanRenderEngineBackend>(m_framework);
		m_ui->addBackend(m_uiRenderingBackend);

		// buildCommandBuffers();
	}

	catch (vk::SystemError& err)
	{
		throw std::runtime_error(err.what());
	}

	catch (std::runtime_error& err)
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
	if (width == 0 || height == 0)
		return;

	m_framework->initializeFramebuffer(width, height);

	buildCommandBuffers();
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::cleanup()
{
	m_framework->m_graphicQueue.waitIdle();
	m_framework->m_device->waitIdle();
}

void GraphicEngine::Vulkan::VulkanRenderingEngine::buildCommandBuffers()
{
	std::array<vk::ClearValue, 3> clearValues;

	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ backgroudColor.r, backgroudColor.g, backgroudColor.b, backgroudColor.a }));
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0.0f);
	clearValues[2].color = vk::ClearColorValue(std::array<float, 4>({ backgroudColor.r, backgroudColor.g, backgroudColor.b, backgroudColor.a }));
	int i{ 0 };
	m_framework->m_device->waitIdle();

	m_ui->nextFrame();
	m_ui->drawUi();

	for (auto& commandBuffer : m_framework->m_commandBuffers)
	{
		commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

		commandBuffer->setViewport(0, vk::Viewport(0.0f, static_cast<float>(m_framework->m_swapChainData.extent.height),
			static_cast<float>(m_framework->m_swapChainData.extent.width), -static_cast<float>(m_framework->m_swapChainData.extent.height), 0.0f, 1.0f));
		commandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_framework->m_swapChainData.extent));

		vk::RenderPassBeginInfo renderPassBeginInfo(m_framework->m_renderPass.get(), m_framework->m_frameBuffers[i].get(), vk::Rect2D(vk::Offset2D(0, 0), m_framework->m_swapChainData.extent), static_cast<uint32_t>(clearValues.size()), clearValues.data());
		commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		m_normalDebugGraphicPipeline->draw(commandBuffer, i);
		m_wireframeGraphicPipeline->draw(commandBuffer, i);
		m_solidColorraphicPipeline->draw(commandBuffer, i);
		

		m_uiRenderingBackend->renderData(commandBuffer);

		commandBuffer->endRenderPass();

		commandBuffer->end();
		++i;
	}
}
