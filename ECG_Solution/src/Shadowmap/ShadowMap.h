#pragma once
#include "../Shader.h"
#include "../Camera.h"


class ShadowMap {
private:
	Shader* shader;
	GLuint shadowMapFbo;
	GLuint shadowMap;
	glm::vec3 lightPos;
	glm::mat4 lightSpaceMatrix;
	float near_plane;
	float far_plane;
	float range;
	const unsigned int SHADOW_MAP_SIZE = 1024;
public:
	ShadowMap(Shader* shader, glm::vec3 lightPos, float near_plane, float far_plane, float range);
	~ShadowMap();
	void initBuffer();
	void generateShadowMap();
	void ConfigureShaderAndMatrices();
	void draw(Shader* shader, Camera& camera, glm::mat4 modelMatrix);
	void drawDebug(Shader* shader);
	void unbindFBO();
	void updateLightPos(glm::vec3 lightPos);
	GLuint getShadowMapID();
	glm::vec3 getLightPos();
	glm::mat4 getLightSpaceMatrix();
};