/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include "INIReader.h"
#include <iostream>
#include <memory>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

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


/* --------------------------------------------- */
// Framework functions
/* --------------------------------------------- */

bool initFramework();
void destroyFramework();
void setMeshColor(glm::vec3 color);