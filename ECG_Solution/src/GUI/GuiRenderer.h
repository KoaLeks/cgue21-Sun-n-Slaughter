#pragma once
#include "../Mesh.h"
#include "GuiTexture.h"
#include "../Query.h"

#include <glm/gtc/matrix_transform.hpp>


class GuiRenderer {
private:
	Mesh quad = Mesh(glm::mat4(1), Mesh::createQuadMesh());
	Shader* shader;
	glm::mat4 transformationMatrix;
	Query query;
	const float scale = 0.75;
	// total pixels = (scale / 16 * 1600)^2 * 4; (4x Multisample)
	const float total_samples = pow(scale * 100, 2) * 4;
	float occlusionFactor;
public:
	GuiRenderer(Shader* shader);
	GuiRenderer();
	~GuiRenderer();
	void initQuery();
	void setShader(Shader* shader);
	void render(std::vector<GuiTexture> guis);
	void doOcclusionTest(glm::vec2 sunPos);
	glm::mat4 calculateTransformationMatrix(glm::vec2 pos, glm::vec2 scale);
	void renderFlares(std::vector<GuiTexture> guis, float brightness, glm::vec2 sunScreenPos);
};