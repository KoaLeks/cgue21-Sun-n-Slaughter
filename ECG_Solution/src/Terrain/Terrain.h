#pragma once
#include "..\stb_image.h"
#include "..\Mesh.h"
#include "TerrainShader.h"
#include "../Camera.h"
#include "../Shadowmap/ShadowMap.h"

class Terrain {
private:

	MeshData data;
	glm::mat4 _modelMatrix;
	const char* heightMapPath;
	const char* normalMapPath;
	const char* waterTexturePath;
	const char* sandTexturePath;
	const char* grassTexturePath;
	const char* stoneTexturePath;
	const char* snowTexturePath;
	int width, height;
	int terrainCount;
	float scaleXZ;
	float scaleY;

	GLuint terrainVbo;
	GLuint terrainVboNorm;
	GLuint terrainEbo;
	GLuint terrainVao;
	GLuint heightMap;
	GLuint waterTexture;
	GLuint sandTexture;
	GLuint grassTexture;
	GLuint stoneTexture;
	GLuint snowTexture;

public:

	Terrain();
	Terrain(int dimension, int vertexCount, float height, const char* heightMapPath, bool shadowMap);
	~Terrain();

	void generateTerrain(int dimension, int vertexCount);
	void generateTerrainTriangleMesh(int dimesion, int vertexCount);
	void draw(TerrainShader* terrainShader, Camera& camera, ShadowMap& shadowMap);
	void draw(Shader* shader);
	void initBuffer();
	void loadHeightMap();
	void loadTexture(GLuint& texture, const char* texturePath);
	glm::mat4 getModelMatrix();
};