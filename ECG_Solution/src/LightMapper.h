#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include <vector>
#include "Geometry.h"
#include "Light.h"


class LightMapper
{
private:
	GLuint _computeShader;
	GLuint pos, norm, uv, idx;
	GLuint texture;
	GLuint lightTexturePos, widthPos, heightPos, pointL1Color, pointL1Position, pointL1Atten, sizePos, materialPos;
	GLuint pointL2Color, pointL2Position, pointL2Atten;
	GLuint WIDTH = 256;
	GLuint HEIGHT = 256;
	PointLight _pointL1;
	PointLight _pointL2;

public:
	LightMapper(GLuint computeShader) :
		_computeShader(computeShader) {}
	void init(PointLight pointL1, PointLight pointL2);
	GLuint calculateLightMap(GeometryData &data, std::shared_ptr<Material> mat);


	~LightMapper();
};

