#include "SettingsWindow.hpp"

GraphicEngine::GUI::SettingWindow::SettingWindow(std::shared_ptr<ViewportSettingWindow> settingWindow, std::shared_ptr<CameraManagerWindow> cameraManagerWindow, std::shared_ptr<LightManagerWindow> lightManager)
{
	m_body = std::make_shared<WindowBody>("Settings");
	m_body->addChildren(settingWindow);
	m_body->addChildren(cameraManagerWindow);
	m_body->addChildren(lightManager);
}

void GraphicEngine::GUI::SettingWindow::draw()
{
	m_body->draw();
}