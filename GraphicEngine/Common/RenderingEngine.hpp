#pragma once

#define NOMINMAX

#include "../Core/EventManager.hpp"
#include "../Core/Logger.hpp"
#include "Vertex.hpp"
#include "Camera.hpp"
#include "../Scene/Resources/Model.hpp"
#include "../Modules/Assimp/AssimpModelImporter.hpp"
#include "../Core/Utils/ObjectConverter.hpp"

#include "../Engines/Graphic/3D/ObjectGenerator.hpp"
#include "../Engines//Graphic/Shaders/Models/Light.hpp"

namespace GraphicEngine
{
	class RenderingEngine
	{
	public:
		RenderingEngine(std::shared_ptr<Common::Camera> camera,
			std::shared_ptr<Core::EventManager> eventManager,
			std::shared_ptr<Core::Configuration> cfg) :
			m_camera(camera),
			m_eventManager(eventManager),
			m_cfg(cfg)
		{
			auto cylinder = Engines::Graphic::CylinderGenerator<Common::VertexPN>{}.getModel(glm::vec3(0.0f), 0.5f, 0.5f, 2.0f, glm::ivec3(20, 2, 2), Engines::Graphic::TriangleDirection::CounterClockwise);
			m_models.push_back(cylinder);
			m_models.front()->setScale(m_cfg->getProperty<float>("scene:object:scale"));
			m_models.front()->setRotate(Core::Utils::Converter::fromArrayToObject<glm::vec3, std::vector<float>, 3>(m_cfg->getProperty<std::vector<float>>("scene:object:rotate")));
			m_models.front()->setPosition(Core::Utils::Converter::fromArrayToObject<glm::vec3, std::vector<float>, 3>(m_cfg->getProperty<std::vector<float>>("scene:object:position")));

			m_models.front()->getMeshes().front()->setRotate(Core::Utils::Converter::fromArrayToObject<glm::vec3, std::vector<float>, 3>(m_cfg->getProperty<std::vector<float>>("scene:object:rotate")));
			m_models.front()->applyTransformation();
			
			light.lightPosition = glm::vec3(100.0, 100.0, 100.0);
		}

		virtual bool drawFrame() = 0;
		virtual void init(size_t width, size_t height) = 0;
		virtual void resizeFrameBuffer(size_t width, size_t height) = 0;
		virtual void cleanup() = 0;

		void setCamera(std::shared_ptr<Common::Camera> camera) { m_camera = camera; }

		virtual ~RenderingEngine() = default;
	protected:
		std::shared_ptr<Common::Camera> m_camera;
		std::shared_ptr<Core::EventManager> m_eventManager;
		std::shared_ptr<Core::Configuration> m_cfg;

		std::vector<std::shared_ptr<Scene::Model<Common::VertexPN>>> m_models;
		Engines::Graphic::Shaders::Light light;
	};
}

