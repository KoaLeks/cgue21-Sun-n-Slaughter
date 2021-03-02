/*
* Copyright 2018 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/

#include "Mesh.h"
#include "stb_image.h"

Mesh::Mesh(glm::mat4 modelMatrix, MeshData& data, std::shared_ptr<MeshMaterial> material, const char* _path)
	: _modelMatrix(modelMatrix), _elements(data.indices.size()), _material(material), path(_path)
{
	// create VAO
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// create positions VBO
	glGenBuffers(1, &_vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, _vboPositions);
	glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec3), data.positions.data(), GL_STATIC_DRAW);

	// bind positions to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// create normals VBO
	glGenBuffers(1, &_vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, _vboNormals);
	glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec3), data.normals.data(), GL_STATIC_DRAW);

	// bind normals to location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	////create uv vbo
	glGenBuffers(1, &_vboUV);
	glBindBuffer(GL_ARRAY_BUFFER, _vboUV);
	glBufferData(GL_ARRAY_BUFFER, data.uv_coord.size() * sizeof(glm::vec2), data.uv_coord.data(), GL_STATIC_DRAW);

	//bind uv to location 2
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


	// create and bind indices VBO
	glGenBuffers(1, &_vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);
	//loadHeightMap();
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &_vboPositions);
	glDeleteBuffers(1, &_vboNormals);
	glDeleteBuffers(1, &_vboIndices);
	glDeleteVertexArrays(1, &_vao);
}

void Mesh::loadHeightMap() {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	glGenTextures(1, &heightmap);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_R16, GL_UNSIGNED_SHORT, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load image" << std::endl;
	}
	stbi_image_free(data);
}
void Mesh::draw()
{
	Shader* shader = _material->getShader();
	shader->use();

	shader->setUniform("modelMatrix", _modelMatrix);
	shader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(_modelMatrix))));


	//glBindTexture(GL_TEXTURE_2D, heightmap);
	//shader->setUniform("heightMap", 0);
	glBindVertexArray(_vao);
	//_material->setUniforms();
	glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->unuse();
}

void Mesh::transform(glm::mat4 transformation)
{
	_modelMatrix = transformation * _modelMatrix;
}

void Mesh::resetModelMatrix()
{
	_modelMatrix = glm::mat4(1);
}

MeshData Mesh::createPlaneMesh(int dimensions) {

	MeshData data;
	int half = dimensions / 2;
	int count = 0;
	for (int width = 0; width < dimensions; width++)
	{
		for (int height = 0; height < dimensions; height++)
		{
			glm::vec3 pos = glm::vec3(height - half, 0, width - half);
			data.positions.push_back(pos);
			data.normals.push_back(glm::vec3(0, 1, 0));

			if (width < (dimensions - 1) || height < (dimensions - 1)) {
				data.indices.push_back(dimensions * width + height + dimensions);
				data.indices.push_back(dimensions * width + height + dimensions + 1);
				data.indices.push_back(dimensions * width + height + 1);

				data.indices.push_back(dimensions * width + height + 1);
				data.indices.push_back(dimensions * width + height);
				data.indices.push_back(dimensions * width + height + dimensions);
			}
		}
	}
	return std::move(data);
}
MeshData Mesh::createCubeMesh(float width, float height, float depth)
{
	MeshData data;

	data.positions = {
		// front
		glm::vec3(-width / 2.0f, -height / 2.0f,  depth / 2.0f), //bottom level
		glm::vec3(width / 2.0f, -height / 2.0f,  depth / 2.0f), //bottom right
		glm::vec3(width / 2.0f, height / 2.0f,  depth / 2.0f), //top right
		glm::vec3(-width / 2.0f, height / 2.0f,  depth / 2.0f), //top left
		// back
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, height / 2.0f,  -depth / 2.0f),
		// right
		glm::vec3(width / 2.0f, -height / 2.0f,  depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, height / 2.0f,  depth / 2.0f),
		// left
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,  depth / 2.0f),
		glm::vec3(-width / 2.0f, height / 2.0f,  depth / 2.0f),
		glm::vec3(-width / 2.0f, height / 2.0f,  -depth / 2.0f),
		// top
		glm::vec3(-width / 2.0f, height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, height / 2.0f,  depth / 2.0f),
		glm::vec3(width / 2.0f, height / 2.0f,  depth / 2.0f),
		glm::vec3(width / 2.0f, height / 2.0f,  -depth / 2.0f),
		// bottom
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,  depth / 2.0f)
	};



	for (int i = 0; i < 6; i++)
	{
		data.uv_coord.push_back(glm::vec2(0, 0));
		data.uv_coord.push_back(glm::vec2(1, 0));
		data.uv_coord.push_back(glm::vec2(1, 1));
		data.uv_coord.push_back(glm::vec2(0, 1));
	}



	data.normals = {
		// front
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		// back
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		// right
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		// left
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		// top
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		// bottom
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0)
	};


	data.indices = {
		// front
		0, 1, 2,
		2, 3, 0,
		// back
		4, 5, 6,
		6, 7, 4,
		// right
		8, 9, 10,
		10, 11, 8,
		// left
		12, 13, 14,
		14, 15, 12,
		// top
		16, 17, 18,
		18, 19, 16,
		// bottom
		20, 21, 22,
		22, 23, 20
	};

	return std::move(data);
}


MeshData Mesh::createSkyboxMesh(float width, float height, float depth) {
	MeshData data;
	// orginal: back, left, right, front, top, bottom 
	data.positions = {				



		// back
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f), //top left 
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f), //bottom left
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f), //bottom right
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f), //bottom right
		glm::vec3(width / 2.0f,  height / 2.0f,  -depth / 2.0f), //top right
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f), //top left
		// left
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f), 
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f), 
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f), 
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f), 
		glm::vec3(-width / 2.0f,  height / 2.0f,   depth / 2.0f), 
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f),
		// right
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f,  height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),

		// front
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f),
		glm::vec3(-width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,   depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f),
		// top
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f),
		glm::vec3( width / 2.0f,  height / 2.0f,  -depth / 2.0f),
		glm::vec3( width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3( width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(-width / 2.0f,  height / 2.0f,   depth / 2.0f),
		glm::vec3(-width / 2.0f,  height / 2.0f,  -depth / 2.0f),

		// bottom
		glm::vec3(-width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,  -depth / 2.0f),
		glm::vec3(-width / 2.0f, -height / 2.0f,   depth / 2.0f),
		glm::vec3(width / 2.0f, -height / 2.0f,   depth / 2.0f)


	};



	//for (int i = 0; i < 6; i++)
	//{
	//	data.uv_coord.push_back(glm::vec2(0, 0));
	//	data.uv_coord.push_back(glm::vec2(1, 0));
	//	data.uv_coord.push_back(glm::vec2(1, 1));
	//	data.uv_coord.push_back(glm::vec2(0, 1));
	//}



	//data.normals = {
	//	// front
	//	glm::vec3(0, 0, 1),
	//	glm::vec3(0, 0, 1),
	//	glm::vec3(0, 0, 1),
	//	glm::vec3(0, 0, 1),
	//	// back
	//	glm::vec3(0, 0, -1),
	//	glm::vec3(0, 0, -1),
	//	glm::vec3(0, 0, -1),
	//	glm::vec3(0, 0, -1),
	//	// right
	//	glm::vec3(1, 0, 0),
	//	glm::vec3(1, 0, 0),
	//	glm::vec3(1, 0, 0),
	//	glm::vec3(1, 0, 0),
	//	// left
	//	glm::vec3(-1, 0, 0),
	//	glm::vec3(-1, 0, 0),
	//	glm::vec3(-1, 0, 0),
	//	glm::vec3(-1, 0, 0),
	//	// top
	//	glm::vec3(0, 1, 0),
	//	glm::vec3(0, 1, 0),
	//	glm::vec3(0, 1, 0),
	//	glm::vec3(0, 1, 0),
	//	// bottom
	//	glm::vec3(0, -1, 0),
	//	glm::vec3(0, -1, 0),
	//	glm::vec3(0, -1, 0),
	//	glm::vec3(0, -1, 0)
	//};

	data.indices = {
		// front
		0, 1, 2,
		3, 4, 5,
		// back
		6, 7, 8,
		9, 10, 11,
		// right
		12, 13, 14,
		14, 15, 16,
		// left
		17, 18, 19,
		20, 21, 22,
		// top
		23, 24, 25,
		26, 27, 28,
		// bottom
		29, 30, 31,
		32, 33, 34
	};

	return std::move(data);
}


MeshData Mesh::createCylinderMesh(unsigned int segments, float height, float radius)
{
	MeshData data;


	// center vertices
	data.positions.push_back(glm::vec3(0, -height / 2.0f, 0));
	data.normals.push_back(glm::vec3(0, -1, 0));
	data.uv_coord.push_back(glm::vec2(0.5f, 0.5f));

	data.positions.push_back(glm::vec3(0, height / 2.0f, 0));
	data.normals.push_back(glm::vec3(0, 1, 0));
	data.uv_coord.push_back(glm::vec2(0.5f, 0.5f));

	// circle segments
	float angle_step = 2.0f * glm::pi<float>() / float(segments);
	for (unsigned int i = 0; i < segments; i++) {
		glm::vec3 circlePos = glm::vec3(
			glm::cos(i * angle_step) * radius,
			-height / 2.0f,
			glm::sin(i * angle_step) * radius
		);

		// bottom ring vertex
		data.positions.push_back(circlePos);
		data.positions.push_back(circlePos);
		data.normals.push_back(glm::vec3(0, -1, 0));
		data.normals.push_back(glm::normalize(circlePos - glm::vec3(0, -height / 2.0f, 0)));
		data.uv_coord.push_back(glm::vec2(circlePos.x, circlePos.z) / radius + 0.5f);
		data.uv_coord.push_back(glm::vec2(float(i) / segments, 0));

		// top ring vertex
		circlePos.y = height / 2.0f;
		data.positions.push_back(circlePos);
		data.positions.push_back(circlePos);
		data.normals.push_back(glm::vec3(0, 1, 0));
		data.normals.push_back(glm::normalize(circlePos - glm::vec3(0, height / 2.0f, 0)));
		data.uv_coord.push_back(glm::vec2(circlePos.x, circlePos.z) / radius + 0.5f);
		data.uv_coord.push_back(glm::vec2(float(i) / segments, 1));

		// bottom face
		data.indices.push_back(0);
		data.indices.push_back(2 + i * 4);
		data.indices.push_back(i == segments - 1 ? 2 : 2 + (i + 1) * 4);

		// top face
		data.indices.push_back(1);
		data.indices.push_back(i == segments - 1 ? 4 : (i + 2) * 4);
		data.indices.push_back((i + 1) * 4);

		// side faces
		data.indices.push_back(3 + i * 4);
		data.indices.push_back(i == segments - 1 ? 5 : 5 + (i + 1) * 4);
		data.indices.push_back(i == segments - 1 ? 3 : 3 + (i + 1) * 4);

		data.indices.push_back(i == segments - 1 ? 5 : 5 + (i + 1) * 4);
		data.indices.push_back(3 + i * 4);
		data.indices.push_back(5 + i * 4);
	}

	return std::move(data);
}

MeshData Mesh::createSphereMesh(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius)
{
	MeshData data;

	data.positions.push_back(glm::vec3(0.0f, radius, 0.0f));
	data.positions.push_back(glm::vec3(0.0f, -radius, 0.0f));

	data.normals.push_back(glm::vec3(0.0f, radius, 0.0f));
	data.normals.push_back(glm::vec3(0.0f, -radius, 0.0f));

	data.uv_coord.push_back(glm::vec2(0, 1));
	data.uv_coord.push_back(glm::vec2(0, 0));

	// first and last ring
	for (unsigned int j = 0; j < longitudeSegments; j++) {
		data.indices.push_back(0);
		data.indices.push_back(j == longitudeSegments - 1 ? 2 : (j + 3));
		data.indices.push_back(2 + j);

		data.indices.push_back(2 + (latitudeSegments - 2) * longitudeSegments + j);
		data.indices.push_back(j == longitudeSegments - 1 ? 2 + (latitudeSegments - 2) * longitudeSegments : 2 + (latitudeSegments - 2) * longitudeSegments + j + 1);
		data.indices.push_back(1);
	}

	// vertices and rings
	for (unsigned int i = 1; i < latitudeSegments; i++) {
		float verticalAngle = float(i) * glm::pi<float>() / float(latitudeSegments);
		for (unsigned int j = 0; j < longitudeSegments; j++) {
			float horizontalAngle = float(j) * 2.0f * glm::pi<float>() / float(longitudeSegments);
			glm::vec3 position = glm::vec3(
				radius * glm::sin(verticalAngle) * glm::cos(horizontalAngle),
				radius * glm::cos(verticalAngle),
				radius * glm::sin(verticalAngle) * glm::sin(horizontalAngle)
			);
			data.positions.push_back(position);

			data.normals.push_back(glm::normalize(position));

			data.uv_coord.push_back(glm::vec2(float(j) / longitudeSegments, float(i) / latitudeSegments));
			//data.uv_coord.push_back(glm::vec2(float(i) / latitudeSegments, float(j) / longitudeSegments));

			if (i == 1) continue;

			data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
			data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
			data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 1) * longitudeSegments : 2 + (i - 1) * longitudeSegments + j + 1);

			data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
			data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
			data.indices.push_back(2 + (i - 2) * longitudeSegments + j);
		}
	}


	return std::move(data);
}


