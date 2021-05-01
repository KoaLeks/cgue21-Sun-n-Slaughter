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



struct GeometryData {
	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
};


class Geometry
{
protected:

	GLuint _vao;
	GLuint _vboPositions;
	GLuint _vboNormals;
	GLuint _vboUVs;
	GLuint _vboIndices;
	unsigned int _elements;

	std::shared_ptr<Material> _material;

	glm::mat4 _modelMatrix;
	glm::mat4 _transformMatrix;

	bool _isEmpty;
	std::vector<std::unique_ptr<Geometry>> _children;

public:
	Geometry(glm::mat4 modelMatrix, GeometryData& data, std::shared_ptr<Material> material);
	Geometry(glm::mat4 modelMatrix = glm::mat4(1.0f));

	~Geometry();

	void draw(glm::mat4 matrix = glm::mat4(1.0f));

	void transform(glm::mat4 transformation);
	void setTransformMatrix(glm::mat4 transformMatrix);
	void resetModelMatrix();

	Geometry* addChild(std::unique_ptr<Geometry> child);

	static GeometryData createCubeGeometry(float width, float height, float depth);
	static GeometryData createCylinderGeometry(unsigned int segments, float height, float radius);
	static GeometryData createSphereGeometry(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius);
};