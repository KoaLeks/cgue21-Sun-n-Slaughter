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

glm::vec3 FlareManager::convertToScreenSpace(glm::mat4 viewProjMatrix, glm::vec3 sunPos) {
	glm::vec4 coords = viewProjMatrix * glm::vec4(sunPos, 1.0f);
	if (coords.w <= 0) {
		return glm::vec3(0, 0, coords.w);
	}
	coords.x /= coords.w;
	coords.y /= coords.w;
	return glm::vec3(coords.x, coords.y, coords.w);
}

void FlareManager::render(glm::mat4 viewProjMatrix, glm::vec3 sunPos) {
	glm::vec3 sunCoords = convertToScreenSpace(viewProjMatrix, sunPos);
	if (sunCoords.z <= 0) {
		return;
	}
	glm::vec2 sunToCenter = screen_center - glm::vec2(sunCoords.x, sunCoords.y);
	float brightness = 1 - (glm::length(sunToCenter));
	if (brightness > 0) {
		calcFlarePos(sunToCenter, sunCoords);
		renderer.renderFlares(flares, brightness, sunCoords);
	}
}