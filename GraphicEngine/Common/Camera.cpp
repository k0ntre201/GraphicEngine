#include "Camera.hpp"
#include "../Core/Ranges.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 GraphicEngine::Common::Camera::getViewProjectionMatrix()
{
	return _viewProjectionMatrix;
}

glm::mat4 GraphicEngine::Common::Camera::getViewMatrix()
{
	if (_shouldUpdateView)
		updateViewMatrix();
	return _viewMatrix;
}

glm::mat4 GraphicEngine::Common::Camera::getProjectionMatrix()
{
	if (_shouldUpdateProjection)
		updateProjectionMatrix();
	return _projectionMatrix;
}

void GraphicEngine::Common::Camera::setCameraPerspectiveProperties(PerspectiveParameters perspectiveParameters)
{
	_perspectiveParameters = perspectiveParameters;
	_cameraType = CameraType::Perspective;
	_shouldUpdateProjection = true;
}

void GraphicEngine::Common::Camera::setCameraOrthogonalProperties(OrthogonalParameters orthogonalParameters)
{
	_orthogonalParameters = orthogonalParameters;
	_cameraType = CameraType::Orthogonal;
	_shouldUpdateProjection = true;
}

void GraphicEngine::Common::Camera::setSpeed(float speed)
{
	_speed = speed;
}

float GraphicEngine::Common::Camera::getSpeed()
{
	return _speed;
}

void GraphicEngine::Common::Camera::setSensitivity(float sensitivity)
{
	_sensitivity = sensitivity;
}

float GraphicEngine::Common::Camera::getSensitivity()
{
	return _sensitivity;
}

void GraphicEngine::Common::Camera::setFOV(float fov)
{
	_perspectiveParameters.fov = fov;
	_shouldUpdateProjection = true;
}

void GraphicEngine::Common::Camera::setAspectRatio(float aspectRatio)
{
	_perspectiveParameters.aspectRatio = aspectRatio;
	_shouldUpdateProjection = _shouldUpdateProjection;
}

void GraphicEngine::Common::Camera::setCameraType(CameraType cameraType)
{
	_cameraType = cameraType;
	_shouldUpdateProjection = true;
}

GraphicEngine::Common::Camera::Camera()
{
}

GraphicEngine::Common::Camera::Camera(PerspectiveParameters perspectiveParameters) :
	_perspectiveParameters(perspectiveParameters), _cameraType(CameraType::Perspective)
{
}

GraphicEngine::Common::Camera::Camera(OrthogonalParameters orthogonalParameters):
	_orthogonalParameters(orthogonalParameters), _cameraType(CameraType::Orthogonal)
{
}

void GraphicEngine::Common::Camera::rotate(const glm::vec2& offset)
{
	_yawPitchOffset = (offset * _sensitivity);
	float oldPitch = _yawPitch.y;
	_yawPitch += _yawPitchOffset;
	
	if (_yawPitch.x > 360.0f || _yawPitch.x < -360.0)
		_yawPitch.x = 0.0f;

	if (_yawPitch.y > 89.0f || _yawPitch.y < -89.0)
	{
		_yawPitch.y = oldPitch;
		_yawPitchOffset.y = 0.0f;
	}

	_shouldUpdateView = true;
}

void GraphicEngine::Common::Camera::move(const glm::vec2& offset)
{
	_position = _position + (_direction * offset.x * _speed);
	_position = _position + (glm::normalize(glm::cross(_direction, glm::vec3(0.0f, 1.0f, 0.0f)) * offset.x * _speed));
	_shouldUpdateView = true;
}

glm::mat4 GraphicEngine::Common::Camera::caclulatePerspective()
{
	return glm::perspective(glm::radians(_perspectiveParameters.fov), _perspectiveParameters.aspectRatio, _perspectiveParameters.zNear, _perspectiveParameters.zFar);
}

glm::mat4 GraphicEngine::Common::Camera::calculateOrthogonal()
{
	return glm::ortho(_orthogonalParameters.left, _orthogonalParameters.right, _orthogonalParameters.bottom, _orthogonalParameters.top);
}

void GraphicEngine::Common::Camera::updateViewMatrix()
{
	glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 right = glm::normalize(glm::cross(_direction, up));
	glm::quat yawQuat = glm::angleAxis(glm::radians(_yawPitchOffset.x), up);
	glm::quat pitchQuat = glm::angleAxis(glm::radians(_yawPitchOffset.y), right);
	glm::quat rot = glm::normalize(glm::cross(yawQuat, pitchQuat));
	_direction = glm::normalize(glm::rotate(rot, _direction));
	_up = glm::normalize(glm::cross(right, _direction));

	_viewMatrix = glm::lookAt(_position, _position + _direction, _up);

	_yawPitchOffset = glm::vec2(0.0f, 0.0f);
	_shouldUpdateView = false;
}

void GraphicEngine::Common::Camera::updateProjectionMatrix()
{
	_projectionMatrix = calculateProjectionMatrix();
	_shouldUpdateProjection = false;
}

GraphicEngine::Common::CameraController::CameraController(std::shared_ptr<Camera> camera):
	_camera(camera)
{
}

void GraphicEngine::Common::CameraController::setCameraType(CameraType cameraType)
{
	_camera->setCameraType(cameraType);
}

void GraphicEngine::Common::CameraController::setDt(float dt)
{
	_dt = dt;
}

void GraphicEngine::Common::CameraController::setInitialMousePosition(float x, float y)
{
	_prevMousePosition = glm::vec2(x, y);
}

void GraphicEngine::Common::CameraController::rotate(float x, float y, const std::vector<GraphicEngine::Core::Inputs::MouseButton>& buttons)
{
	glm::vec2 newOffset = _prevMousePosition - glm::vec2(x, y);
	if (_rotateButton == Core::Inputs::MouseButton::buttonNone || std::find(std::begin(buttons), std::end(buttons), _rotateButton) != std::end(buttons))
	{
		_camera->rotate(newOffset * _dt);
		_prevMousePosition += newOffset;
	}
}

void GraphicEngine::Common::CameraController::move(std::vector<GraphicEngine::Core::Inputs::KeyboardKey> keys)
{
	using namespace Core::Inputs;
	std::vector<KeyboardKey> basicMovementKeys{ KeyboardKey::KEY_W, KeyboardKey::KEY_A, KeyboardKey::KEY_S, KeyboardKey::KEY_D };
	std::vector<KeyboardKey> filteredKeys = GameEngine::Core::Ranges::filter(keys, [&](KeyboardKey key) 
		{
			return std::find(std::begin(basicMovementKeys), std::end(basicMovementKeys), key) != std::end(basicMovementKeys); 
		});
	glm::vec2 movementOffset{ 0.0,0.0 };
	for (KeyboardKey key : filteredKeys)
	{
		if (key == KeyboardKey::KEY_W)
			movementOffset.x += _dt;
		else if (key == KeyboardKey::KEY_S)
			movementOffset.x -= _dt;
		else if (key == KeyboardKey::KEY_A)
			movementOffset.y -= _dt;
		else if (key == KeyboardKey::KEY_D)
			movementOffset.y += _dt;
	}

	_camera->move(movementOffset);
}
