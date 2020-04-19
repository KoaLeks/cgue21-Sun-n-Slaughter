#pragma once
#include "TerrainShader.h"
#include <sstream>


TerrainShader::~TerrainShader() {}

TerrainShader::TerrainShader(std::string vs, std::string tc, std::string te, std::string fs) {

	
	// 1. retrieve the vertex/tessellation/fragment source code from filePath
	std::string vertexCode;
	std::string tessellationControlCode;
	std::string tessellationEvaluationCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream tcShaderFile;
	std::ifstream teShaderFile;
	std::ifstream fShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vs);
		tcShaderFile.open(tc);
		teShaderFile.open(te);
		fShaderFile.open(fs);
		std::stringstream vShaderStream, tcShaderStream, teShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		tcShaderStream << tcShaderFile.rdbuf();
		teShaderStream << teShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		tcShaderFile.close();
		teShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		tessellationControlCode = tcShaderStream.str();
		tessellationEvaluationCode = teShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* tcShaderCode = tessellationControlCode.c_str();
    const char* teShaderCode = tessellationEvaluationCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

	// 2. compile shaders
	unsigned int vertex, tessellationControl, tessellationEvaluation, fragment;
	int success;
	char infoLog[512];

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	};
	
	// tessellation control Shader
	tessellationControl = glCreateShader(GL_TESS_CONTROL_SHADER);
	glShaderSource(tessellationControl, 1, &tcShaderCode, NULL);
	glCompileShader(tessellationControl);
	// print compile errors if any
	glGetShaderiv(tessellationControl, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(tessellationControl, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::TESS_CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	// tessellation evaluation Shader
	tessellationEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
	glShaderSource(tessellationEvaluation, 1, &teShaderCode, NULL);
	glCompileShader(tessellationEvaluation);
	// print compile errors if any
	glGetShaderiv(tessellationEvaluation, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(tessellationEvaluation, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::TESS_EVAL::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, tessellationControl);
	glAttachShader(ID, tessellationEvaluation);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	// print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertex);
	glDeleteShader(tessellationControl);
	glDeleteShader(tessellationEvaluation);
	glDeleteShader(fragment);
}


void TerrainShader::setUniform(std::string uniform, const int i) {
	glUniform1i(getUniformLocation(uniform), i);
}

void TerrainShader::setUniform(std::string uniform, const float f) {
	glUniform1f(getUniformLocation(uniform), f);
}

void TerrainShader::setUniform(std::string uniform, const glm::mat4& mat) {
	glUniformMatrix4fv(getUniformLocation(uniform), 1, false, glm::value_ptr(mat));
}
void TerrainShader::setUniform(std::string uniform, const glm::vec3& vec) {
	glUniform3fv(getUniformLocation(uniform), 1, glm::value_ptr(vec));
}
void TerrainShader::setUniform(std::string uniform, const glm::vec2& vec) {
	glUniform2fv(getUniformLocation(uniform), 1, glm::value_ptr(vec));
}
GLint TerrainShader::getUniformLocation(std::string uniform) {
	return glGetUniformLocation(ID, uniform.c_str());
}
void TerrainShader::use() {
	glUseProgram(ID);
}