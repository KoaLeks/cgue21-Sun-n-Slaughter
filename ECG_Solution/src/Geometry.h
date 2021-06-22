/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include <vector>
#include <memory>
#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>

#include <glm\gtc\matrix_transform.hpp>
#include "Shader.h"
#include "Material.h"

/* GAMEPLAY */
#include "FrustumG.h"
#include <PxPhysicsAPI.h>
/* GAMEPLAY END */




struct GeometryData {
	std::vector<glm::vec4> positions;
	std::vector<unsigned int> indices;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> uvs;
};


class Geometry
{
protected:

	/* GAMEPLAY */
	physx::PxController* _pxChar;
	std::shared_ptr<std::vector<glm::vec3>> _boudingBox;
	std::shared_ptr<FrustumG> _viewFrustum;
	unsigned int* _drawnObjects;
	/* GAMEPLAY END*/

	//GLuint _vao;
	//GLuint _vboPositions;
	//GLuint _vboNormals;
	//GLuint _vboUVs;
	//GLuint _vboIndices;
	//unsigned int _elements;

	std::shared_ptr<Material> _material;

	glm::mat4 _modelMatrix;
	glm::mat4 _transformMatrix;

	bool _isEmpty;
	std::vector<std::shared_ptr<Geometry>> _children;

public:

	/* GAMEPLAY */
	GLuint _vao;
	GLuint _vboPositions;
	GLuint _vboNormals;
	GLuint _vboUVs;
	GLuint _vboIndices;
	unsigned int _elements;

	GLuint vector_size;
	bool _isCharacter = false;
	physx::PxRigidActor* _actor;

	physx::PxController* getCharacterController();
	physx::PxRigidActor* getActor();
	/* GAMEPLAY END*/

	Geometry(glm::mat4 modelMatrix, GeometryData& data, std::shared_ptr<Material> material, physx::PxRigidActor* actor, physx::PxController* pxChar, std::shared_ptr<std::vector<glm::vec3>> boundingBox, std::shared_ptr<FrustumG> viewFrustum, unsigned int* drawnObjects);
	Geometry(glm::mat4 modelMatrix, GeometryData& data, std::shared_ptr<Material> material);
	Geometry(glm::mat4 modelMatrix = glm::mat4(1.0f));

	~Geometry();

	void draw(glm::mat4 matrix = glm::mat4(1.0f));

	void draw(Shader* shader, glm::mat4 matrix);

	void transform(glm::mat4 transformation);
	void setTransformMatrix(glm::mat4 transformMatrix);
	void resetModelMatrix();
	void updateBoundingBox(glm::vec3 posDelta);
	void drawDebug(glm::mat4 matrix, std::string name);
	Geometry* addChild(std::shared_ptr<Geometry> child);
};