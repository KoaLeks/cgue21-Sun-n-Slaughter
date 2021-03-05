#pragma once
#include "Terrain.h"

Terrain::Terrain(int dimension, int vertexCount, float height, const char* heightMapPath, bool shadowMap) {

	this->heightMapPath = heightMapPath;
	this->scaleXZ = dimension;
	this->scaleY = height;
	if (shadowMap) {
		this->generateTerrainTriangleMesh(dimension, vertexCount);
	}
	else
	{
		this->generateTerrain(dimension, vertexCount);
	}
	//this->loadHeightMap();
	this->loadHeightMap();
	//this->loadTexture(heightMap, heightMapPath);
	this->loadTexture(waterTexture, "assets/terrain/textures/water.jpg");
	this->loadTexture(sandTexture, "assets/terrain/textures/sand.jpg");
	this->loadTexture(grassTexture, "assets/terrain/textures/grass.jpg");
	this->loadTexture(stoneTexture, "assets/terrain/textures/stone.jpg");
	this->loadTexture(snowTexture, "assets/terrain/textures/snow.jpg");
	this->initBuffer();
}

Terrain::~Terrain() {
	glDeleteBuffers(1, &terrainVbo);
	glDeleteBuffers(1, &terrainVboNorm);
	glDeleteBuffers(1, &terrainEbo);
	glDeleteVertexArrays(1, &terrainVao);
}

glm::mat4 Terrain::getModelMatrix() {
	return _modelMatrix;
}

void Terrain::generateTerrainTriangleMesh(int dimension, int vertexCount) {

	// actual width, height
	const int w = dimension, h = dimension;

	// number of vertices pro zeile/spalte
	const int verticesWidth = vertexCount, verticesHeight = vertexCount;
	int i = 0;

	for (int z = 0; z < verticesHeight; z++) {
		for (int x = 0; x < verticesWidth; x++)
		{
			// pos mapped von 0 bis 1
			glm::vec3 pos = glm::vec3((x) / (float)verticesWidth, -1000, (z) / (float)verticesHeight);

			pos.x *= w;
			pos.x -= w / 2;
			pos.z *= h;
			pos.z -= h / 2;

			data.positions.push_back(pos);
			data.normals.push_back(glm::vec3(0, 1, 0));

			// quads: top left, top right, bottom right, bottom left
			if (((i + 1) % verticesWidth) != 0 && z + 1 < verticesHeight) {

				data.indices.push_back(i);
				data.indices.push_back(i + verticesWidth);
				data.indices.push_back(i + verticesWidth + 1);

				data.indices.push_back(i + verticesWidth + 1);
				data.indices.push_back(i + 1);
				data.indices.push_back(i);

			}
			i++;
		}
	}
	terrainCount = data.indices.size();

}

void Terrain::generateTerrain(int dimension, int vertexCount) {
	
	// actual width, height
	const int w = dimension, h = dimension;

	// number of vertices pro zeile/spalte
	const int verticesWidth = vertexCount, verticesHeight = vertexCount;
	int i = 0;

	for (int z = 0; z < verticesHeight; z++) {
		for (int x = 0; x < verticesWidth; x++)
		{
			// pos mapped von 0 bis 1
			glm::vec3 pos = glm::vec3((x) /(float)verticesWidth, -1000, (z) /(float)verticesHeight);
			
			pos.x *= w;
			pos.x -= w/2;
			pos.z *= h;
			pos.z -= h/2;
			
			data.positions.push_back(pos);
			data.normals.push_back(glm::vec3(0, 1, 0));
			
			// quads: top left, top right, bottom right, bottom left
			if (((i + 1) % verticesWidth) != 0 && z + 1 < verticesHeight) {
				data.indices.push_back(i);
				data.indices.push_back(i + 1);
				data.indices.push_back(i + verticesWidth + 1);
				data.indices.push_back(i + verticesWidth);
			}
			i++;
		}
	}
	terrainCount = data.indices.size();

}
void Terrain::initBuffer() {
	// create VAO
	glGenVertexArrays(1, &terrainVao);
	glBindVertexArray(terrainVao);
	
	// create positions VBO
	glGenBuffers(1, &terrainVbo);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * data.positions.size(), data.positions.data(), GL_STATIC_DRAW);

	// bind positions to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// create normals VBO
	glGenBuffers(1, &terrainVboNorm);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVboNorm);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * data.normals.size(), data.normals.data(), GL_STATIC_DRAW);

	// bind normals to location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// create indicies VBO
	glGenBuffers(1, &terrainEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * data.indices.size(), data.indices.data(), GL_STATIC_DRAW);


	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Terrain::loadHeightMap() {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(heightMapPath, &width, &height, &nrChannels, 0);

	glGenTextures(1, &heightMap);
	glBindTexture(GL_TEXTURE_2D, heightMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if (data) {

		// heightmap = GL_RGB, heihtmap10 = GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load image" << std::endl;
	}
	stbi_image_free(data);
}

void Terrain::loadTexture(GLuint& texture, const char* texturePath) {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if (data) {

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load image" << std::endl;
	}
	stbi_image_free(data);
}

void Terrain::draw(TerrainShader* terrainShader, Camera& camera, ShadowMap& shadowMap) {
	terrainShader->use();

	terrainShader->setUniform("modelMatrix", _modelMatrix);
	terrainShader->setUniform("scaleXZ", scaleXZ);
	terrainShader->setUniform("scaleY", scaleY);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap);
	terrainShader->setUniform("heightMap", 0);

	// terrain textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterTexture);
	terrainShader->setUniform("waterTexture", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sandTexture);
	terrainShader->setUniform("sandTexture", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	terrainShader->setUniform("grassTexture", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, stoneTexture);
	terrainShader->setUniform("stoneTexture", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snowTexture);
	terrainShader->setUniform("snowTexture", 5);

	// shadowMap
	terrainShader->setUniform("lightSpaceMatrix", shadowMap.getLightSpaceMatrix());
	terrainShader->setUniform("lightPos", shadowMap.getLightPos());
	terrainShader->setUniform("viewPos", camera.getPosition());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, shadowMap.getShadowMapID());
	terrainShader->setUniform("shadowMap", 6);

	glBindVertexArray(terrainVao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, terrainCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	terrainShader->unuse();
}

void Terrain::draw(Shader* shader) {
	shader->use();
	shader->setUniform("modelMatrix", _modelMatrix);
	shader->setUniform("scaleXZ", scaleXZ);
	shader->setUniform("scaleY", scaleY);
	shader->setUniform("isTerrain", true);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap);
	shader->setUniform("heightMap", 0);

	glBindVertexArray(terrainVao);
	//glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_TRIANGLES, terrainCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->unuse();
}