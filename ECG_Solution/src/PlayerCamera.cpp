
#include "PlayerCamera.h"


PlayerCamera::PlayerCamera(float fov, float aspect, float near, float far) {
	_projMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
	_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, _zoom));
	_rotationMatrix = glm::rotate(glm::rotate(glm::mat4(1.0f), _yaw, glm::vec3(0.0f, 1.0f, 0.0f)), _pitch, glm::vec3(1.0f, 0.0f, 0.0f));
}

PlayerCamera::~PlayerCamera() {
}

glm::vec3 PlayerCamera::getPosition() {
	return _position;
}

glm::mat4 PlayerCamera::getViewProjectionMatrix() {
	return _projMatrix * _viewMatrix * glm::transpose(_rotationMatrix) * glm::translate(glm::mat4(1.0f), _position);
}

void PlayerCamera::updateZoom(float zoom) {
	if (_zoom != zoom) {
		_zoom = zoom;
		_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, _zoom));
	}
}

void PlayerCamera::rotate(float xRotate, float yRotate) {
	_yaw += xRotate;
	_pitch += yRotate;


	if (_pitch < MINUS_90_DEGREE) {
		_pitch = MINUS_90_DEGREE;
	}
	if (_pitch > PLUS_90_DEGREE) {
		_pitch = PLUS_90_DEGREE;
	}
	_rotationMatrix = glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
}


void PlayerCamera::move(float forward, float strafeLeft) {
	_position.z += forward;
	_position.x += strafeLeft;
}

void PlayerCamera::setPosition(physx::PxExtendedVec3 pos) {
	_position.x = -pos.x;
	_position.y = -pos.y;
	_position.z = -pos.z;

}

float PlayerCamera::getYaw() {
	return _yaw;
}

glm::mat4 PlayerCamera::getProjection() {
	return _projMatrix;
}

glm::mat4 PlayerCamera::getModel() {
	return _viewMatrix * glm::transpose(_rotationMatrix) * glm::translate(glm::mat4(1.0f), _position);
}