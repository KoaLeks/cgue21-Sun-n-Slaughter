#pragma once
#include "GuiTexture.h"

GuiTexture::GuiTexture(GLuint _textureId, glm::vec2 _position, glm::vec2 _scale) 
	: textureId(_textureId), position(_position), scale(_scale) {

}

GLuint GuiTexture::getTextureId() {
	return textureId;
}
glm::vec2 GuiTexture::getPosition() {
	return position;
}
glm::vec2 GuiTexture::getScale() {
	return scale;
}