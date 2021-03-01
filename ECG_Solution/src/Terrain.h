#pragma once
#include "Mesh.h"
#include "TerrainShader.h"
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
	GLuint normalMap;
	GLuint waterTexture;
	GLuint sandTexture;
	GLuint grassTexture;
	GLuint stoneTexture;
	GLuint snowTexture;
	std::shared_ptr<MeshMaterial> _material;

public:

	Terrain();
	
	Terrain(int dimension, int vertexCount, float height, const char* heightMapPath, const char* normalMapPath);

	~Terrain();

	void generateTerrain(int dimension, int vertexCount);

	void draw(TerrainShader* terrainShader);
	
	void draw(Shader* shader);

	void initBuffer();

	void loadHeightMap();

	void loadNormalMap();

	void loadTexture(GLuint& texture, const char* texturePath);

};