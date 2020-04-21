#pragma once
#include "Mesh.h"
#include "TerrainShader.h"
class Terrain {
private:

	MeshData data;
	glm::mat4 _modelMatrix;
	const char* path;
	int width, height;
	int terrainCount;
	float scaleXZ;
	float scaleY;

	GLuint terrainVbo;
	GLuint terrainVboNorm;
	GLuint terrainEbo;
	GLuint terrainVao;
	GLuint heightmap;
	std::shared_ptr<MeshMaterial> _material;

public:

	Terrain();
	
	Terrain(int dimension, int vertexCount, float height, const char* path);

	~Terrain();

	void generateTerrain(int dimension, int vertexCount);

	void draw(TerrainShader* terrainShader);
	
	void draw(Shader* shader);

	void initBuffer();

	void loadHeightMap();

};