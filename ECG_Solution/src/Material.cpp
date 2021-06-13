/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#include "Material.h"

/* --------------------------------------------- */
// Base material
/* --------------------------------------------- */



Material::Material(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient)
	: _shader(shader), _materialCoefficients(materialCoefficients), _alpha(specularCoefficient)
{
}

Material::~Material()
{
}

Shader* Material::getShader()
{
	return _shader.get();
}

void Material::setUniforms()
{
	_shader->setUniform("materialCoefficients", _materialCoefficients);
	_shader->setUniform("specularAlpha", _alpha);
}

/* --------------------------------------------- */
// Texture material
/* --------------------------------------------- */

TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient, /*std::shared_ptr<Texture>*/ GLuint diffuseTexture)
	: Material(shader, materialCoefficients, specularCoefficient), _diffuseTexture(diffuseTexture)
{
}

/* GAMEPLAY */
TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient, const char* diffuseTexturePath)
	: Material(shader, materialCoefficients, specularCoefficient) {
	_diffuseTexture = loadTextureFromFile(diffuseTexturePath);
}
/* GAMEPLAY END */

TextureMaterial::~TextureMaterial()
{
}

//void TextureMaterial::setUniforms()
//{
//	Material::setUniforms();
//
//	_diffuseTexture->bind(0);
//	_shader->setUniform("diffuseTexture", 0);
//}

/* GAMEPLAY */
void TextureMaterial::setUniforms()
{
	Material::setUniforms();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _diffuseTexture);
	_shader->setUniform("diffuseTexture", 0);
	_shader->setUniform("hasLightmap", hasLightmap);

	if (hasLightmap) {
		glActiveTexture(GL_TEXTURE1);
		//glBindImageTexture(1, _lightmapTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindTexture(GL_TEXTURE_2D, _lightmapTexture);
		_shader->setUniform("lightTexture", 1);
	}
}


void Material::setLightmap(GLuint lightmap) {
	_lightmapTexture = lightmap;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _lightmapTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	hasLightmap = true;
}
/* GAMEPLAY END */

