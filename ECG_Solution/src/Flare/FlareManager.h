#pragma once
#include "../GUI/GuiRenderer.h"

class FlareManager {
private:
	GuiRenderer renderer;
	std::vector<GuiTexture> flares;
	const glm::vec2 screen_center = glm::vec2(0.0f);
	float spacing;
public:
	FlareManager(Shader* shader, float _spacing, std::vector<GuiTexture> _flares);
	~FlareManager();
	void calcFlarePos(glm::vec2 sunToCenter, glm::vec2 sunCoords);
	glm::vec3 convertToScreenSpace(glm::mat4 viewProjMatrix, glm::vec3 sunPos);
	void render(glm::mat4 viewProjMatrix, glm::vec3 sunPos, float overallBrightness);
};
