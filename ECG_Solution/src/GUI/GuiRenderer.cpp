#include "GuiRenderer.h"

GuiRenderer::GuiRenderer(Shader* shader) {
	this->shader = shader;
}

GuiRenderer::GuiRenderer() {}

GuiRenderer::~GuiRenderer() {}

void GuiRenderer::setShader(Shader* shader) {
	this->shader = shader;
}

void GuiRenderer::render(std::vector<GuiTexture> guis) {
	shader->use();
	glBindVertexArray(quad.getVaoID());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	shader->setUniform("brightness", 1.0f);
	for (GuiTexture& gui : guis) {
		transformationMatrix = calculateTransformationMatrix(gui.getPosition(), gui.getScale());
		shader->setUniform("transformationMatrix", transformationMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gui.getTextureId());
		shader->setUniform("guiTexture", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, quad.getVertexCount());
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	shader->unuse();
};

void GuiRenderer::render(std::vector<GuiTexture> guis, float brightness) {
	shader->use();
	glBindVertexArray(quad.getVaoID());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	shader->setUniform("brightness", brightness);
	for (GuiTexture& gui : guis) {
		transformationMatrix = calculateTransformationMatrix(gui.getPosition(), gui.getScale());
		shader->setUniform("transformationMatrix", transformationMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gui.getTextureId());
		shader->setUniform("guiTexture", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, quad.getVertexCount());
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	shader->unuse();
};

glm::mat4 GuiRenderer::calculateTransformationMatrix(glm::vec2 pos, glm::vec2 scale) {
	return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), glm::vec3(scale, 1.0f));
}