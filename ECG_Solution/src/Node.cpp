#include "Node.h"




Node::Node(glm::mat4 modelMatrix)
	: _modelMatrix(modelMatrix)
{
	_startingPosition = glm::vec3(0.0f, 0.0f, 0.0f);
}

Node::~Node()
{
}

void Node::draw(glm::mat4 matrix)
{
	if (_enabled) {
		glm::mat4 accumModel = matrix * glm::translate(glm::mat4(1), _position) * glm::rotate(glm::mat4(1), glm::radians(_angle), glm::vec3(0, 1, 0)) * glm::translate(glm::mat4(1), _startingPosition) * _transformMatrix * _modelMatrix;

		for (size_t i = 0; i < _meshes.size(); i++) {
			_meshes[i]->draw(accumModel);
		}

		for (size_t i = 0; i < _children.size(); i++) {
			_children[i]->draw(accumModel);
		}
	}
}


void Node::drawDepth(Shader* shader, glm::mat4 matrix)
{
	if (_enabled) {
		glm::mat4 accumModel = matrix * glm::translate(glm::mat4(1), _position) * glm::rotate(glm::mat4(1), glm::radians(_angle), glm::vec3(0, 1, 0)) * glm::translate(glm::mat4(1), _startingPosition) * _transformMatrix * _modelMatrix;

		for (size_t i = 0; i < _meshes.size(); i++) {
			_meshes[i]->draw(shader, accumModel);
		}

		for (size_t i = 0; i < _children.size(); i++) {
			_children[i]->drawDepth(shader, accumModel);
		}
	}
}

void Node::transform(glm::mat4 transformation)
{
	_modelMatrix = transformation * _modelMatrix;
}

void Node::setTransformMatrix(glm::mat4 transformMatrix)
{
	_transformMatrix = transformMatrix;
}

void Node::resetModelMatrix()
{
	_modelMatrix = glm::mat4(1);
}

void Node::addChild(std::shared_ptr<Node> child)
{
	_children.push_back(std::move(child));
}


void Node::addMesh(std::shared_ptr<Geometry> mesh) {
	_meshes.push_back(mesh);
}

void Node::move(float forward, float strafeLeft) {
	_position.z += forward;
	_position.x += strafeLeft;
}

void Node::setPosition(physx::PxExtendedVec3 pos) {
	_position.x = pos.x;
	_position.y = pos.y;
	_position.z = pos.z;

}

glm::vec3 Node::getPosition() {
	return _position;
}

void Node::yaw(float angle) {
	_angle = angle;
}

std::shared_ptr<Node> Node::getChildWithName(std::string name) {
	for (size_t i = 0; i < _children.size(); i++) {
		if (_children[i]->name == name) {
			return _children[i];
		}
	}
	return nullptr;
}