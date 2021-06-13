#pragma once


#include <vector>
#include <memory>
#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "Geometry.h"
#include <PxPhysicsAPI.h>


class Node
{
protected:
	glm::mat4 _modelMatrix;
	glm::mat4 _transformMatrix;
	bool _enabled = true;

	glm::vec3 _position;
	float _angle = 0.0f;

	std::vector<std::shared_ptr<Node>> _children;

public:
	Node(glm::mat4 modelMatrix = glm::mat4(1.0f));
	~Node();

	glm::vec3 _startingPosition;

	std::string name;
	std::vector<std::shared_ptr<Geometry>> _meshes;

	void draw(glm::mat4 matrix = glm::mat4(1.0f));

	void transform(glm::mat4 transformation);
	void setTransformMatrix(glm::mat4 transformMatrix);
	void resetModelMatrix();

	void addChild(std::shared_ptr<Node> child);
	void addMesh(std::shared_ptr<Geometry> mesh);

	void move(float forward, float strafeLeft);
	void setPosition(physx::PxExtendedVec3 pos);
	void yaw(float angle);
	std::shared_ptr<Node> getChildWithName(std::string name);
};