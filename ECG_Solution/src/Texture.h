/*
* Copyright 2017 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include <string>
#include <GL/glew.h>
#include "Utils.h"
#include "stb_image.h"


/* --------------------------------------------- */
// 2D texture
/* --------------------------------------------- */

class Texture
{
protected:
	GLuint _handle;
	bool _init;
	bool isTransparent;
	float aspectRatio;

public:
	/*!
	 * Creates a texture from a file
	 * @param file: path to the texture file (a DSS image)
	 */
	Texture(std::string file);

	/*!
	 * Creates a texture from a file
	 * @param file: path to the texture file (png/jpg)
	 */
	Texture(const char* texturePath, bool isTransparent);
	Texture();
	~Texture();

	/*!
	 * Activates the texture unit and binds this texture
	 * @param unit: the texture unit
	 */
	void bind(int unit);

	/*!
	 * Returns handle for this texture
	 */
	GLuint getTextureId();

	/*!
	 * Returns aspect ratio for this texture
	 */
	float getAspectRatio();

	void loadTexture(const char* texturePath);
};
