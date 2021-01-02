#pragma once

#define NOMINMAX

#include "../Core/EventManager.hpp"
#include "../Core/Logger.hpp"
#include "Vertex.hpp"
#include "Camera.hpp"
#include "UI.hpp"
#include "../Scene/Resources/Model.hpp"
#include "../Modules/Assimp/AssimpModelImporter.hpp"
#include "../Core/Utils/ObjectConverter.hpp"
#include "../Services/CameraControllerManager.hpp"
#include "../Services/ModelManager.hpp"

#include "../Engines//Graphic/Shaders/Models/Light.hpp"

#include "../UI/ImGui/Widgets/ColorEdit.hpp"
#include "../UI/ImGui/Widgets/Checkbox.hpp"

namespace GraphicEngine
{
	class RenderingEngine
	{
	public:
		RenderingEngine(std::shared_ptr<Services::CameraControllerManager> cameraControllerManager,
			std::shared_ptr<Services::ModelManager> modelManager,
			std::shared_ptr<Core::EventManager> eventManager,
			std::shared_ptr<Common::UI> ui,
			std::shared_ptr<Core::Configuration> cfg);

		virtual bool drawFrame() = 0;
		virtual void init(size_t width, size_t height) = 0;
		virtual void resizeFrameBuffer(size_t width, size_t height) = 0;
		virtual void cleanup() = 0;

		virtual ~RenderingEngine() = default;
	protected:
		std::shared_ptr<Services::CameraControllerManager> m_cameraControllerManager;
		std::shared_ptr<Services::ModelManager> m_modelManager;
		std::shared_ptr<Core::EventManager> m_eventManager;
		std::shared_ptr<Common::UI> m_ui;
		std::shared_ptr<Core::Configuration> m_cfg;

		glm::vec4 backgroudColor{ 0.2f, 0.2f, 0.2f, 1.0f };

		bool displayWireframe{ true };
		bool displaySolid{ true };
		bool displayNormal{ true };
	};
}
