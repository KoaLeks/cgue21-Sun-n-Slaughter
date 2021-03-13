#include "FlareManager.h"

FlareManager::FlareManager(Shader* shader, float _spacing, std::vector<GuiTexture> _flares) 
	: spacing(_spacing), flares(_flares) {
	this->renderer.setShader(shader);
}
FlareManager::~FlareManager() {}

void FlareManager::calcFlarePos(glm::vec2 sunToCenter, glm::vec2 sunCoords) {
	for (int i = 0; i < flares.size(); i++) {
		glm::vec2 direction = sunToCenter * (i * spacing);
		glm::vec2 flarePos = direction + sunCoords;
		flares[i].setPosition(flarePos);
	}
}

glm::vec2 FlareManager::convertToScreenSpace(glm::mat4 viewProjMatrix, glm::vec3 worldPos) {
	glm::vec4 coords = viewProjMatrix * glm::vec4(worldPos, 1.0f);
	coords.x /= coords.w;
	coords.y /= coords.w;
	return glm::vec2(coords.x, coords.y);
}

void FlareManager::render(glm::mat4 viewProjMatrix, glm::vec3 sunPos) {
	glm::vec2 sunCoords = convertToScreenSpace(viewProjMatrix, sunPos);
	glm::vec2 sunToCenter = screen_center - sunCoords;
	float brightness = 1 - (glm::length(sunToCenter) / 0.7f);
	if (brightness > 0) {
		calcFlarePos(sunToCenter, sunCoords);
		renderer.render(flares, brightness);
	}
}