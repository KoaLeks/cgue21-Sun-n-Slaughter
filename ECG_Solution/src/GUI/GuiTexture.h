#pragma once
#include <glm\glm.hpp>
#include <GL\glew.h>

class GuiTexture {
private:
	GLuint textureId;
	glm::vec2 position;
	glm::vec2 scale;

	
public:
	GuiTexture(GLuint textureId, glm::vec2 position, glm::vec2 scale);
	GLuint getTextureId();
	glm::vec2 getPosition();
	glm::vec2 getScale();
	void setPosition(glm::vec2 pos);
};