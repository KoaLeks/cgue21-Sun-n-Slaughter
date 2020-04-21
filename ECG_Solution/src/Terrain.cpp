#pragma once
#include "Terrain.h"
#include "stb_image.h"

Terrain::Terrain(int dimension, int vertexCount, float height, const char* path) {

	this->path = path;
	this->scaleXZ = dimension;
	this->scaleY = height;
	this->generateTerrain(dimension, vertexCount);
	this->loadHeightMap();
	this->initBuffer();
}

Terrain::~Terrain() {
	glDeleteBuffers(1, &terrainVbo);
	glDeleteBuffers(1, &terrainVboNorm);
	glDeleteBuffers(1, &terrainEbo);
	glDeleteVertexArrays(1, &terrainVao);
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
			glm::vec3 pos = glm::vec3((x) /(float)verticesWidth, 0, (z) /(float)verticesHeight);
			
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
	
	glGenBuffers(1, &terrainVbo);
	glGenBuffers(1, &terrainVboNorm);
	glGenBuffers(1, &terrainEbo);

	glBindBuffer(GL_ARRAY_BUFFER, terrainVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * data.positions.size(), data.positions.data(), GL_STATIC_DRAW);

	// bind positions to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, terrainVboNorm);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * data.normals.size(), data.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * data.indices.size(), data.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Terrain::loadHeightMap() {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	glGenTextures(1, &heightmap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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

void Terrain::draw(TerrainShader* terrainShader) {
	terrainShader->use();

	terrainShader->setUniform("modelMatrix", _modelMatrix);
	terrainShader->setUniform("scaleXZ", scaleXZ);
	terrainShader->setUniform("scaleY", scaleY);

	//terrainShader->setUniform("ambientMaterial", glm::vec3( 0.4, 0.1, 0.1));
	//terrainShader->setUniform("diffuseMaterial", glm::vec3( 0.1, 0.7, 0.1));
	//terrainShader->setUniform("specularMeterial", glm::vec3(0.1, 0.1, 0.1));
	//terrainShader->setUniform("shininess", 2.0f);
	
	//terrainShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(_modelMatrix))));
	
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, heightmap);
	//terrainShader->setUniform("heightMap", 0);
	
	glBindVertexArray(terrainVao);
	//_material->setUniforms();
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, terrainCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
