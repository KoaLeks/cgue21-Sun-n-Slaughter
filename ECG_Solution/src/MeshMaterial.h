/*
* Copyright 2018 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainShader.h"


/*!
 * Base material
 */
class MeshMaterial
{
protected:
	/*!
	 * The shader used for rendering this material
	 */
	std::shared_ptr<Shader> _shader;
	std::shared_ptr<TerrainShader> _terrainShader;
	/*!
	 * The material's color
	 */
	glm::vec3 _color;
	/*!
	 * The material's coefficients (x = ambient, y = diffuse, z = specular)
	 */
	glm::vec3 _materialCoefficients;

	/*!
	 * Alpha value, i.e. the shininess constant
	 */
	float _alpha;

	std::shared_ptr<Texture> texture;

public:

	MeshMaterial(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Texture>& texture,
		const glm::vec3& highp_vec3, float alpha);

	MeshMaterial(const std::shared_ptr<Shader>& shader, const glm::vec3& color, float alpha);
	
	MeshMaterial(const std::shared_ptr<TerrainShader>& shader, const std::shared_ptr<Texture>& texture,
			const glm::vec3& highp_vec3, float alpha);

	void set_texture(const std::shared_ptr<Texture>& texture)//TODO: Change to Constructor
	{
		this->texture = texture;
	}

	virtual ~MeshMaterial();

	/*!
	 * @return The shader associated with this material
	 */
	Shader* getShader();
	/*!
	 * @return The shader associated with this material
	 */
	TerrainShader* getTerrainShader();

	/*!
	 * Sets this material's parameters as uniforms in the shader
	 */
	virtual void setUniforms();
};
