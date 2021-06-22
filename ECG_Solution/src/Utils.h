/*
	* Copyright 2017 Vienna University of Technology.
	* Institute of Computer Graphics and Algorithms.
	* This file is part of the ECG Lab Framework and must not be redistributed.
	*/
#pragma once


#include "INIReader.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

/* GAMEPLAY */
#include <FreeImagePlus.h>
#include <vector>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
/* GAMEPLAY */

#define EXIT_WITH_ERROR(err) \
		std::cout << "ERROR: " << err << std::endl; \
		system("PAUSE"); \
		return EXIT_FAILURE;

#define FOURCC_DXT1	MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3	MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5	MAKEFOURCC('D', 'X', 'T', '5')


class DDSImage {
public:
	unsigned char* image;
	unsigned int width;
	unsigned int height;
	unsigned int size;
	GLenum format;

	DDSImage() : image(nullptr), width(0), height(0), size(0), format(GL_NONE) {}
	DDSImage(const DDSImage& img) = delete;
	DDSImage(DDSImage&& img) : image(img.image), width(img.width), height(img.height), size(img.size), format(img.format) {
		img.image = nullptr;
	}
	DDSImage& operator=(const DDSImage& img) = delete;
	DDSImage& operator=(DDSImage&& img) {
		image = img.image;
		img.image = nullptr;
		width = img.width;
		height = img.height;
		size = img.size;
		format = img.format;
		return *this;
	};

	~DDSImage() { if (image != nullptr) { delete[] image; image = nullptr; } }
};

DDSImage loadDDS(const char* file);

/* GAMEPLAY */
GLuint loadTextureFromFile(const char* filename);
GLuint getComputeShader(char* computeshadersource);
GLuint getParticleShader(char* vertexShaderSource, char* geometryShaderSource, char* fragmentShaderSource);
char* filetobuf(char* file);
void enableBlendMode();
void disableBlendMode();
glm::vec3 getLookVector(glm::mat4 modelView);
glm::vec3 getUpVector(glm::mat4 modelView);
glm::vec3 getWorldPosition(glm::mat4 modelView);

int getIntFromCString(const char* str);
/* GAMEPLAY END */

/* --------------------------------------------- */
// Framework functions
/* --------------------------------------------- */

bool initFramework();
void destroyFramework();
void setMeshColor(glm::vec3 color);