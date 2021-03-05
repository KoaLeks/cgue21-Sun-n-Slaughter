#include "ShadowMap.h"

ShadowMap::ShadowMap(Shader* shader, glm::vec3 lightPos, float near_plane, float far_plane, float range) {
	this->shader = shader;
	this->lightPos = lightPos;
	this->near_plane = near_plane;
	this->far_plane = far_plane;
	this->range = range;
	this->initBuffer();
	//this->generateShadowMap();
}

ShadowMap::~ShadowMap() {
	glDeleteFramebuffers(1, &shadowMapFbo);
}

GLuint ShadowMap::getShadowMapID() {
	return shadowMap;
}

glm::mat4 ShadowMap::getLightSpaceMatrix() {
	return lightSpaceMatrix;
}

glm::vec3 ShadowMap::getLightPos() {
	return lightPos;
}

void ShadowMap::initBuffer() {

	// create FBO
	glGenFramebuffers(1, &shadowMapFbo);
	
	// create 2D texture
	glGenTextures(1, &shadowMap); 
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// bind FBO to texture
	// attach texture as the FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::generateShadowMap() {
	// 1. first render to depth map
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	//ConfigureShaderAndMatrices();
}

void ShadowMap::ConfigureShaderAndMatrices() {
	glm::mat4 lightProjection = glm::ortho(-range, range, -range, range, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;

	shader->use();
	shader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	shader->unuse();
}

void ShadowMap::updateLightPos(glm::vec3 lightPos) {
	this->lightPos = lightPos;
}

void ShadowMap::unbindFBO() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::draw(Shader* shader, Camera& camera, glm::mat4 modelMatrix) {
	this->unbindFBO();
	//shader->use();
	//shader->setUniform("modelMatrix", modelMatrix);
	//shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	//shader->setUniform("lightSpaceMatrix", camera.getViewProjectionMatrix());
	//shader->setUniform("lightPos", lightPos);
	//shader->setUniform("viewPos", camera.getPosition());
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, shadowMap);
	//shader->setUniform("shadowMap", shadowMap);

	// reset viewport
	glViewport(0, 0, 1600, 900);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::drawDebug(Shader* shader) {
	this->unbindFBO();
	shader->use();
	shader->setUniform("near_plane", near_plane);
	shader->setUniform("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	shader->setUniform("depthMap", 0);

	// reset viewport
	glViewport(0, 0, 1600, 900);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}