/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"

#include "Texture.h"
#include "Utils.h"


/* --------------------------------------------- */
// Base material
/* --------------------------------------------- */

class Material
{
protected:
	std::shared_ptr<Shader> _shader;

	glm::vec3 _materialCoefficients; // x = ambient, y = diffuse, z = specular
	float _alpha;

public:
	Material(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient);
	virtual ~Material();

	Shader* getShader();
	virtual void setUniforms();

	/* GAMEPLAY */
	GLuint _lightmapTexture;
	bool hasLightmap = false;
	glm::vec3 getCoefficients() {
		return _materialCoefficients;
	}
	/* GAMEPLAY END */

};

/* --------------------------------------------- */
// Texture material
/* --------------------------------------------- */

class TextureMaterial : public Material
{
protected:
	//std::shared_ptr<Texture> _diffuseTexture;
	/* GAMEPLAY */
	GLuint _diffuseTexture;
	/* GAMEPLAY END */
public:
	/* GAMEPLAY */
	TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient, const char* diffuseTexturePath);
	/* GAMEPLAY END */

	TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float specularCoefficient, /*std::shared_ptr<Texture>*/ GLuint diffuseTexture);
	virtual ~TextureMaterial();

	virtual void setUniforms();
	
};

