#include "ParticleRenderer.h"

void ParticleRenderer::init() {
	glUseProgram(_computeShader);

	glGenBuffers(1, &ssbo_pos[0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &ssbo_pos[1]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &ssbo_vel[0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &ssbo_vel[1]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &atomicCounter);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) , &atomic_value);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	float anglePerStep = 360.0f / MAX_PARTICLES;
	float angleParticle = 0;
	for (int i = 0; i < MAX_PARTICLES; i++) {
		positions.push_back(glm::vec4(0.0f, 0.0f, 0.0f, TTL));
		velocities.push_back(glm::vec4(glm::cos(glm::radians(angleParticle)) + glm::sin(-glm::radians(angleParticle)), 0.0f, glm::sin(glm::radians(angleParticle)) + glm::cos(-glm::radians(angleParticle)), 0.0f));
		angleParticle += anglePerStep;
	}

	particle_count = positions.size();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[0]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(positions[0]), &positions[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[0]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(velocities[0]), &velocities[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[1]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(positions[0]), &positions[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[1]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(velocities[0]), &velocities[0]);

	LastCountPos = glGetUniformLocation(_computeShader, "LastCount");
	MaximumCountPos = glGetUniformLocation(_computeShader, "MaximumCount");
	DeltaTPos = glGetUniformLocation(_computeShader, "DeltaT");

	glUseProgram(_renderProgram);

	const GLuint position_layout = 0;
	glGenVertexArrays(2, vaos);
	glBindVertexArray(vaos[0]);
	glBindBuffer(GL_ARRAY_BUFFER, ssbo_pos[0]);
	glEnableVertexAttribArray(position_layout);
	glVertexAttribPointer(position_layout, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(vaos[1]);
	glBindBuffer(GL_ARRAY_BUFFER, ssbo_pos[1]);
	glEnableVertexAttribArray(position_layout);
	glVertexAttribPointer(position_layout, 4, GL_FLOAT, GL_FALSE, 0, 0);

	ModelViewMatrixPos = glGetUniformLocation(_renderProgram, "ModelViewMatrix");
	ProjectionMatrixPos = glGetUniformLocation(_renderProgram, "ProjectionMatrix");

	// set texture
	texture = loadTextureFromFile("assets/textures/smoke.png");
	texturePos = glGetUniformLocation(_renderProgram, "tex0");

	glBindVertexArray(0);
	glUseProgram(0);
}

void ParticleRenderer::calculate(float deltaT) {
	glUseProgram(_computeShader);

	glUniform1ui(LastCountPos, particle_count);
	glUniform1ui(MaximumCountPos, MAX_PARTICLES);
	glUniform1f(DeltaTPos, deltaT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_pos[index]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_vel[index]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_pos[!index]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_vel[!index]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, atomicCounter);

	index = !index;
	GLuint groups = (particle_count / (16 * 16) + 1);
	glDispatchCompute(groups, 1, 1);

	glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

	GLuint buffer_value;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &buffer_value);
	particle_count = buffer_value;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &atomic_value);

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

}

void ParticleRenderer::reset() {
	particle_count = positions.size();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[0]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(positions[0]), &positions[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[0]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(velocities[0]), &velocities[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos[1]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(positions[0]), &positions[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vel[1]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particle_count * sizeof(velocities[0]), &velocities[0]);
}

void ParticleRenderer::draw(glm::mat4 modelView) {
	enableBlendMode();
	glUseProgram(_renderProgram);

	glUniformMatrix4fv(ModelViewMatrixPos, 1, GL_FALSE, glm::value_ptr(modelView));
	glUniformMatrix4fv(ProjectionMatrixPos, 1, GL_FALSE, glm::value_ptr(_perspective));
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(texturePos, 0);
	glBindVertexArray(vaos[index]);
	glDrawArrays(GL_POINTS, 0, particle_count);
	glBindVertexArray(0);
	glUseProgram(0);
	disableBlendMode();
}