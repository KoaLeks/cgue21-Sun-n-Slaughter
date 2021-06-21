/*
	* Copyright 2017 Vienna University of Technology.
	* Institute of Computer Graphics and Algorithms.
	* This file is part of the ECG Lab Framework and must not be redistributed.
	*/
#include "Geometry.h"


Geometry::Geometry(glm::mat4 modelMatrix, GeometryData& data, std::shared_ptr<Material> material, physx::PxRigidActor* actor, physx::PxController* pxChar, std::shared_ptr<std::vector<glm::vec3>> boundingBox, std::shared_ptr<FrustumG> viewFrustum, unsigned int* drawnObjects)
	: _elements(data.indices.size()), _modelMatrix(modelMatrix), _material(material), _actor(actor), _pxChar(pxChar), _boudingBox(boundingBox), _viewFrustum(viewFrustum), _drawnObjects(drawnObjects)
{
	// create VAO
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// create positions VBO
	glGenBuffers(1, &_vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, _vboPositions);
	glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec4), data.positions.data(), GL_STATIC_DRAW);

	// bind positions to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// create normals VBO
	glGenBuffers(1, &_vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, _vboNormals);
	glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec4), data.normals.data(), GL_STATIC_DRAW);

	// bind normals to location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// create uvs VBO
	glGenBuffers(1, &_vboUVs);
	glBindBuffer(GL_ARRAY_BUFFER, _vboUVs);
	glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(glm::vec2), data.uvs.data(), GL_STATIC_DRAW);

	// bind uvs to location 3
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// create and bind indices VBO
	glGenBuffers(1, &_vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	vector_size = data.positions.size();
	_isEmpty = false;
}

Geometry::Geometry(glm::mat4 modelMatrix, GeometryData& data, std::shared_ptr<Material> material)
	: _elements(data.indices.size()), _modelMatrix(modelMatrix), _material(material)
{
	// create VAO
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// create positions VBO
	glGenBuffers(1, &_vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, _vboPositions);
	glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec4), data.positions.data(), GL_STATIC_DRAW);

	// bind positions to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// create normals VBO
	glGenBuffers(1, &_vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, _vboNormals);
	glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec4), data.normals.data(), GL_STATIC_DRAW);

	// bind normals to location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// create uvs VBO
	glGenBuffers(1, &_vboUVs);
	glBindBuffer(GL_ARRAY_BUFFER, _vboUVs);
	glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(glm::vec2), data.uvs.data(), GL_STATIC_DRAW);

	// bind uvs to location 3
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// create and bind indices VBO
	glGenBuffers(1, &_vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	_isEmpty = false;
}

Geometry::Geometry(glm::mat4 modelMatrix)
	: _modelMatrix(modelMatrix), _isEmpty(true)
{
}

Geometry::~Geometry()
{
	if (!_isEmpty) {
		glDeleteBuffers(1, &_vboPositions);
		glDeleteBuffers(1, &_vboNormals);
		glDeleteBuffers(1, &_vboUVs);
		glDeleteBuffers(1, &_vboIndices);
		glDeleteVertexArrays(1, &_vao);
	}
}

void Geometry::draw(glm::mat4 matrix)
{
	glm::mat4 accumModel = matrix * _transformMatrix * _modelMatrix;
	if (_isCharacter || _viewFrustum->boxInFrustum(_boudingBox) != FrustumG::OUTSIDE) {
		if (!_isEmpty) {
			Shader* shader = _material->getShader();
			shader->use();

			shader->setUniform("modelMatrix", accumModel);
			shader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(accumModel))));
			_material->setUniforms();

			glBindVertexArray(_vao);
			glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			(*_drawnObjects)++;
			shader->unuse();

		}
	}

	for (size_t i = 0; i < _children.size(); i++) {
		_children[i]->draw(accumModel);
	}

}

void Geometry::draw(Shader* shader, glm::mat4 matrix)
{
	glm::mat4 accumModel = matrix * _transformMatrix * _modelMatrix;
	if (_isCharacter || _viewFrustum->boxInFrustum(_boudingBox) != 0) {
		if (!_isEmpty) {

			shader->use();

			shader->setUniform("modelMatrix", accumModel);
			shader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(accumModel))));
			shader->setUniform("isTerrain", false);

			glBindVertexArray(_vao);
			glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			(*_drawnObjects)++;

			shader->unuse();
		}
	}

	for (size_t i = 0; i < _children.size(); i++) {
		_children[i]->draw(shader, accumModel);
	}

}

void Geometry::transform(glm::mat4 transformation)
{
	_modelMatrix = transformation * _modelMatrix;
}

void Geometry::setTransformMatrix(glm::mat4 transformMatrix)
{
	_transformMatrix = transformMatrix;
}

void Geometry::resetModelMatrix()
{
	_modelMatrix = glm::mat4(1);
}

Geometry* Geometry::addChild(std::shared_ptr<Geometry> child)
{
	_children.push_back(std::move(child));
	return (_children.end() - 1)->get();
}


physx::PxRigidActor* Geometry::getActor() {
	return _actor;
}

physx::PxController* Geometry::getCharacterController() {
	return _pxChar;
}