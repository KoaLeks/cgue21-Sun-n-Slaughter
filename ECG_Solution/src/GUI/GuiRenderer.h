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
	const float scale = 1;
	int windowWidth;
	float total_samples;
	float occlusionFactor;
public:
	GuiRenderer(Shader* shader);
	GuiRenderer();
	~GuiRenderer();
	void initQuery();
	void setWindowWidth(int width);
	void setShader(Shader* shader);
	void render(std::vector<GuiTexture> guis, float brightness);
	void doOcclusionTest(glm::vec2 sunPos);
	glm::mat4 calculateTransformationMatrix(glm::vec2 pos, glm::vec2 scale);
	void renderFlares(std::vector<GuiTexture> guis, float brightness, glm::vec2 sunScreenPos);
};