#include "LightMapper.h"




LightMapper::~LightMapper()
{
}

void LightMapper::init(PointLight pointL1, PointLight pointL2) {
	glUseProgram(_computeShader);


	lightTexturePos = glGetUniformLocation(_computeShader, "lightTexture");
	widthPos = glGetUniformLocation(_computeShader, "width");
	heightPos = glGetUniformLocation(_computeShader, "height");
	pointL1Color = glGetUniformLocation(_computeShader, "pointL1.color");
	pointL1Position = glGetUniformLocation(_computeShader, "pointL1.position");
	pointL1Atten = glGetUniformLocation(_computeShader, "pointL1.attenuation");
	pointL2Color = glGetUniformLocation(_computeShader, "pointL2.color");
	pointL2Position = glGetUniformLocation(_computeShader, "pointL2.position");
	pointL2Atten = glGetUniformLocation(_computeShader, "pointL2.attenuation");
	sizePos = glGetUniformLocation(_computeShader, "dataSize");
	materialPos = glGetUniformLocation(_computeShader, "materialCoefficients");

	glUseProgram(0);

	_pointL1 = pointL1;
	_pointL2 = pointL2;
}


GLuint LightMapper::calculateLightMap(GeometryData &data, std::shared_ptr<Material> mat) {
	glUseProgram(_computeShader);
	glGenBuffers(1, &pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data.positions.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.positions.size() * sizeof(data.positions[0]), &data.positions[0]);

	glGenBuffers(1, &norm);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, norm);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data.normals.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.normals.size() * sizeof(data.normals[0]), &data.normals[0]);

	glGenBuffers(1, &uv);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uv);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data.uvs.size() * sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.uvs.size() * sizeof(data.uvs[0]), &data.uvs[0]);

	glGenBuffers(1, &idx);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, idx);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data.indices.size() * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.indices.size() * sizeof(data.indices[0]), &data.indices[0]);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, norm);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, uv);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, idx);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUniform1ui(lightTexturePos, 0);
	glUniform1ui(widthPos, WIDTH);
	glUniform1ui(heightPos, HEIGHT);
	glUniform1ui(sizePos, data.indices.size());
	glUniform3f(pointL1Color, _pointL1.color.x, _pointL1.color.y, _pointL1.color.z);
	glUniform3f(pointL1Position, _pointL1.position.x, _pointL1.position.y, _pointL1.position.z);
	glUniform3f(pointL1Atten, _pointL1.attenuation.x, _pointL1.attenuation.y, _pointL1.attenuation.z);
	glUniform3f(pointL2Color, _pointL2.color.x, _pointL2.color.y, _pointL2.color.z);
	glUniform3f(pointL2Position, _pointL2.position.x, _pointL2.position.y, _pointL2.position.z);
	glUniform3f(pointL2Atten, _pointL2.attenuation.x, _pointL2.attenuation.y, _pointL2.attenuation.z);
	glUniform3f(materialPos, 1.0f, 1.0f, 1.0f);

	// heavy lifting done on gpu
	glDispatchCompute(8, 8, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
	return texture;
}
