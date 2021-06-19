#pragma once
#include "Terrain.h"
#include "../PoissonDiskSampling.h"

Terrain::Terrain(int dimension, int vertexCount, float height, const char* heightMapPath, bool shadowMap) {
	this->scaleXZ = dimension;
	this->scaleY = height;
	if (shadowMap) {
		this->generateTerrainTriangleMesh(dimension, vertexCount);
	}
	else
	{
		this->generateTerrain(dimension, vertexCount);
	}
	heightMap.setTransparent(true);
	heightMap.loadTexture(heightMapPath);
	this->initBuffer();
}

Terrain::~Terrain() {
	glDeleteBuffers(1, &terrainVbo);
	glDeleteBuffers(1, &terrainVboNorm);
	glDeleteBuffers(1, &terrainEbo);
	glDeleteVertexArrays(1, &terrainVao);
	GLuint texture = waterTexture.getTextureId();
	glDeleteTextures(1, &texture);
	texture = sandTexture.getTextureId();
	glDeleteTextures(1, &texture);
	texture = grassTexture.getTextureId();
	glDeleteTextures(1, &texture);
	texture = stoneTexture.getTextureId();
	glDeleteTextures(1, &texture);
	texture = snowTexture.getTextureId();
	glDeleteTextures(1, &texture);
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
			//pos.x -= w / 2;
			pos.z *= h;
			//pos.z -= h / 2;
			pos.z -= h;

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
			//pos.x -= w/2;
			pos.z *= h;
			//pos.z -= h/2;
			pos.z -= h;
			
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

void Terrain::draw(TerrainShader* terrainShader, PlayerCamera& camera, ShadowMap& shadowMap, float brightness) {
	terrainShader->use();

	terrainShader->setUniform("modelMatrix", _modelMatrix);
	terrainShader->setUniform("scaleXZ", scaleXZ);
	terrainShader->setUniform("scaleY", scaleY);
	terrainShader->setUniform("brightness", brightness);

	heightMap.bind(0);
	terrainShader->setUniform("heightMap", 0);

	// terrain textures
	waterTexture.bind(1);
	sandTexture.bind(2);
	grassTexture.bind(3);
	stoneTexture.bind(4);
	snowTexture.bind(5);
	terrainShader->setUniform("waterTexture", 1);
	terrainShader->setUniform("sandTexture", 2);
	terrainShader->setUniform("grassTexture", 3);
	terrainShader->setUniform("stoneTexture", 4);
	terrainShader->setUniform("snowTexture", 5);

	terrainShader->setUniform("viewPos", camera.getPosition());

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

	heightMap.bind(0);
	shader->setUniform("heightMap", 0);

	glBindVertexArray(terrainVao);
	glDrawElements(GL_TRIANGLES, terrainCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->unuse();
}