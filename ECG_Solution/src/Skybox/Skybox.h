#pragma once
#include <GL\glew.h>
#include "..\stb_image.h"
#include <iostream>
#include <vector>
#include <glm\detail\type_vec.hpp>
#include <glm\detail\type_mat.hpp>
#include "..\Mesh.h"
#include "..\PlayerCamera.h"

class Skybox {

private:
	Shader* shader;
	GLuint textureID;
	GLuint skyboxVao;
	GLuint skyboxVbo;
	char* textureFaces[6];
	MeshData data;
public:

	Skybox(Shader* shader);
	~Skybox();
	void loadCubemap();
	void initBuffer();
	void draw(PlayerCamera& camera, float brightness);
};