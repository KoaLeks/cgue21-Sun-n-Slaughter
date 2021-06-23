#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include <vector>

class ParticleRenderer
{
private:
	GLuint _computeShader;
	GLuint _renderProgram;
	int MAX_PARTICLES = 50;
	GLuint ssbo_pos[2];
	GLuint ssbo_vel[2];
	GLuint atomicCounter;
	GLfloat TTL = 1.0f;
	GLuint tmp_buffer;
	GLuint atomic_value = 0;

	std::vector<glm::vec4> positions;
	std::vector<glm::vec4> velocities;

	int particle_count;
	GLuint vaos[2];
	GLuint LastCountPos, MaximumCountPos, DeltaTPos, ModelViewMatrixPos, ProjectionMatrixPos;
	int index = 0;
	GLuint texturePos, texture;

	glm::mat4 _perspective;

public:
	ParticleRenderer(GLuint computeShader, GLuint renderProgram, glm::mat4 perspectice)
		: _computeShader(computeShader), _renderProgram(renderProgram), _perspective(perspectice) {
	}
	void init();
	void updateVelocities(glm::vec3 viewDir);
	void calculate(float deltaT);
	void draw(glm::mat4 modelView);
	void reset();
	~ParticleRenderer() {}
};

