#pragma once


#include <memory>

#include <glm\glm.hpp>
#include <glm\gtc/matrix_transform.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <PxPhysicsAPI.h>


class PlayerCamera {
private:
	glm::mat4 _viewMatrix;
	glm::mat4 _projMatrix;
	float _yaw = 0;
	float _pitch = 0;
	float _zoom = -6.0f;
	glm::vec3 _position;

	glm::mat4 _rotationMatrix;
	const float MINUS_90_DEGREE = -90.0f;
	const float PLUS_90_DEGREE = 90.0f;


public:
	PlayerCamera(float fov, float aspect, float near, float far);
	~PlayerCamera();

	glm::vec3 getPosition();
	glm::mat4 getViewProjectionMatrix();
	glm::mat4 getProjection();
	glm::mat4 getModel();
	void updateZoom(float zoom);
	void rotate(float xRotate, float yRotate);
	void move(float forward, float strafeLeft);
	void setPosition(physx::PxExtendedVec3 pos);
	float getYaw();
};