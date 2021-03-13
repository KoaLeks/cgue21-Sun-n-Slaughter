#pragma once
#include "../Mesh.h"
#include "GuiTexture.h"


class GuiRenderer {
private:
	Mesh quad = Mesh(glm::mat4(1), Mesh::createQuadMesh());
	Shader* shader;
	glm::mat4 transformationMatrix;
public:
	GuiRenderer(Shader* shader);
	GuiRenderer();
	~GuiRenderer();
	void setShader(Shader* shader);
	void render(std::vector<GuiTexture> guis);
	void render(std::vector<GuiTexture> guis, float brightness);
	glm::mat4 calculateTransformationMatrix(glm::vec2 pos, glm::vec2 scale);

};