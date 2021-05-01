/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include <GL\glew.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Utils.h"

class Shader
{
protected:
	GLuint _handle;
	std::string _vs, _fs;
	bool _useFileAsSource;

	std::unordered_map<std::string, GLint> _locations;

	GLuint loadShaders();
	bool loadShader_(std::string file, GLenum shaderType, GLuint& handle);
	GLint getUniformLocation(std::string uniform);

public:

	Shader();
	Shader(std::string vs, std::string fs);
	~Shader();

	void use() const;
	void unuse() const;

	void setUniform(std::string uniform, const int i);
	void setUniform(GLint location, const int i);
	void setUniform(std::string uniform, const unsigned int i);
	void setUniform(GLint location, const unsigned int i);
	void setUniform(std::string uniform, const float f);
	void setUniform(GLint location, const float f);
	void setUniform(std::string uniform, const glm::mat4& mat);
	void setUniform(GLint location, const glm::mat4& mat);
	void setUniform(std::string uniform, const glm::mat3& mat);
	void setUniform(GLint location, const glm::mat3& mat);
	void setUniform(std::string uniform, const glm::vec2& vec);
	void setUniform(GLint location, const glm::vec2& vec);
	void setUniform(std::string uniform, const glm::vec3& vec);
	void setUniform(GLint location, const glm::vec3& vec);
	void setUniform(std::string uniform, const glm::vec4& vec);
	void setUniform(GLint location, const glm::vec4& vec);
	void setUniformArr(std::string arr, unsigned int i, std::string prop, const glm::vec3& vec);
	void setUniformArr(std::string arr, unsigned int i, std::string prop, const float f);
};