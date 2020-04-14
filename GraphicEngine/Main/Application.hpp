#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Engine.hpp"

#include <string>

#include "../Common/WindowKeyboardMouse.hpp"
#include "../Core/EventManager.hpp"
#include "../Drivers/OpenGL/OpenGLRenderingEngine.hpp"
#include "../Drivers/Vulkan/VulkanRenderingEngine.hpp"
#include "../Platform/Glfw/OpenGL/GlfwOpenGLWindow.hpp"
#include "../Platform/Glfw/Vulkan/GlfwVulkanWindow.hpp"

class Application
{
public:
	Application(int argc, char** argv);

	void exec();

private:
	/*std::shared_ptr<GraphicEngine::Engine> engine;
	std::shared_ptr<GraphicEngine::Core::Inputs::KeyboardEventProxy> keyboard;
	std::shared_ptr<GraphicEngine::Core::Inputs::MouseEventProxy> mouse;
	std::shared_ptr<GraphicEngine::Common::CameraController> cameraController;
	std::shared_ptr<GraphicEngine::Common::Camera> camera;
	std::shared_ptr<GraphicEngine::Core::EventManager> eventManager;*/
};

#endif // !APPLICATION_HPP

