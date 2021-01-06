#pragma once

#include "../../Common/RenderingEngine.hpp"
#include "../../Core/Logger.hpp"
#include "../../Engines/Graphic/Shaders/Models/ModelMatrices.hpp"

#include "OpenGLShader.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGLUniformBuffer.hpp"
#include "OpenGLVertexBuffer.hpp"
#include "OpenGLShaderStorageBufferObject.hpp"

// Pipelines
#include "GraphicPipelines/OpenGLWireframeGraphicPipeline.hpp"
#include "GraphicPipelines/OpenGLSolidColorGraphicPipeline.hpp"
#include "GraphicPipelines/OpenGLNormalDebugGraphicPileline.hpp"
#include "GraphicPipelines/OpenGLSkyboxGraphicPipeline.hpp"

#include "../../UI/ImGui/ImGuiImpl.hpp"

namespace GraphicEngine::OpenGL
{
	class OpenGLRenderingEngine : public GraphicEngine::RenderingEngine
	{
	public:
		OpenGLRenderingEngine(std::shared_ptr<Services::CameraControllerManager> cameraControllerManager,
			std::shared_ptr<Services::ModelManager> modelManager,
			std::shared_ptr<Services::LightManager> lightManager,
			std::shared_ptr<Core::EventManager> eventManager,
			std::shared_ptr<Common::UI> ui,
			std::shared_ptr<Core::Configuration> cfg,
			std::unique_ptr<Core::Logger<OpenGLRenderingEngine>> logger);
		virtual bool drawFrame() override;
		virtual void init(size_t width, size_t height) override;
		virtual void resizeFrameBuffer(size_t width, size_t height) override;
		virtual void cleanup() override;

		virtual ~OpenGLRenderingEngine() = default;
	private:
		std::shared_ptr<UniformBuffer<Engines::Graphic::Shaders::CameraMatrices>> m_cameraUniformBuffer;
		std::shared_ptr<ShaderStorageBufferObject<Engines::Graphic::Shaders::DirectionalLight>> m_directionalLight;
		std::shared_ptr<ShaderStorageBufferObject<Engines::Graphic::Shaders::PointLight>> m_pointLights;
		std::shared_ptr<ShaderStorageBufferObject<Engines::Graphic::Shaders::SpotLight>> m_spotLight;

		std::unique_ptr<Core::Logger<OpenGLRenderingEngine>> m_logger;

		std::unique_ptr<OpenGLWireframeGraphicPipeline> m_wireframeGraphicPipeline;
		std::unique_ptr<OpenGLSolidColorGraphicPipeline> m_solidColorGraphicPipeline;
		std::unique_ptr<OpenGLNormalDebugGraphicPipeline> m_normalDebugGraphicPipeline;
		std::unique_ptr<OpenGLSkyboxGraphicPipeline> m_skyboxGraphicPipeline;

		std::shared_ptr<GUI::ImGuiImpl::OpenGlRenderEngineBackend> m_uiRenderingBackend;
	};
}
