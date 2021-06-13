#pragma once
#include "../stb_image.h"
#include "../Mesh.h"
#include "TerrainShader.h"
#include "../Camera.h"
#include "../Shadowmap/ShadowMap.h"

class Terrain {
private:

	MeshData data;
	std::vector<glm::vec2> treePositions;
	glm::mat4 _modelMatrix;
	int width, height;
	int terrainCount;
	float scaleXZ;
	float scaleY;

	GLuint terrainVbo;
	GLuint terrainVboNorm;
	GLuint terrainEbo;
	GLuint terrainVao;

	Texture heightMap;
	Texture waterTexture = Texture("assets/terrain/textures/water.jpg", false);
	Texture sandTexture = Texture("assets/terrain/textures/sand.jpg", false);
	Texture grassTexture = Texture("assets/terrain/textures/grass.jpg", false);
	Texture stoneTexture = Texture("assets/terrain/textures/stone.jpg", false);
	Texture snowTexture = Texture("assets/terrain/textures/snow.jpg", false);

public:

	Terrain();
	Terrain(int dimension, int vertexCount, float height, const char* heightMapPath, bool shadowMap);
	~Terrain();

	void generateTerrain(int dimension, int vertexCount);
	void generateTerrainTriangleMesh(int dimesion, int vertexCount);
	void draw(TerrainShader* terrainShader, Camera& camera, ShadowMap& shadowMap, float brightness);
	void draw(Shader* shader);
	void initBuffer();
	glm::mat4 getModelMatrix();
};