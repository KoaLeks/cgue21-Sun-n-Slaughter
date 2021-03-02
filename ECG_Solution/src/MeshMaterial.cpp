/*
* Copyright 2018 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#include "MeshMaterial.h"

/* --------------------------------------------- */
// Base material
/* --------------------------------------------- */


MeshMaterial::MeshMaterial(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Texture>& texture,
	const glm::vec3& highp_vec3, float alpha) : _shader(shader),
	_materialCoefficients(highp_vec3),
	_alpha(alpha),
	texture(texture)
{
}
MeshMaterial::MeshMaterial(const std::shared_ptr<Shader>& shader, const glm::vec3& color, float alpha) : _shader(shader),
	_alpha(alpha),
	_color(color)
{
}
MeshMaterial::MeshMaterial(const std::shared_ptr<TerrainShader>& shader, const std::shared_ptr<Texture>& texture,
	const glm::vec3& highp_vec3, float alpha) : _terrainShader(shader),
	_materialCoefficients(highp_vec3),
	_alpha(alpha),
	texture(texture)
{
}

MeshMaterial::~MeshMaterial()
{
}

Shader* MeshMaterial::getShader()
{
	return _shader.get();
}

TerrainShader* MeshMaterial::getTerrainShader()
{
	return _terrainShader.get();
}

void MeshMaterial::setUniforms()
{
	_terrainShader->setUniform("ambientMaterial",  glm::vec3( _materialCoefficients.x, 0.5, 0.5));
	_terrainShader->setUniform("diffuseMaterial",  glm::vec3( 0.1, _materialCoefficients.y, 0.5));
	_terrainShader->setUniform("specularMeterial", glm::vec3( 0.1, 0.1, _materialCoefficients.z));
	_terrainShader->setUniform("shininess", _alpha);
	texture->bind(0);
}
