
#include "Utils.h"

// https://r3dux.org/2014/10/how-to-load-an-opengl-texture-using-the-freeimage-library-or-freeimageplus-technically/
GLuint loadTextureFromFile(const char* filename) {
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);

	if (format == -1) {
		std::cout << "Could not find image: " << filename << "." << std::endl;
	}

	if (format == FIF_UNKNOWN) {
		std::cout << "Couldn't determine file format - attempting to get from file extension..." << std::endl;
		format = FreeImage_GetFIFFromFilename(filename);

		if (!FreeImage_FIFSupportsReading(format)) {
			std::cout << "Detected image format cannot be read!" << std::endl;
		}
	}


	FIBITMAP* bitmap = FreeImage_Load(format, filename);
	int bitsPerPixel = FreeImage_GetBPP(bitmap);
	FIBITMAP* bitmap32;
	if (bitsPerPixel == 32) {
		bitmap32 = bitmap;
	} else {
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	}

	int imageWidth = FreeImage_GetWidth(bitmap32);
	int imageHeight = FreeImage_GetHeight(bitmap32);

	GLubyte* textureData = FreeImage_GetBits(bitmap32);

	GLuint tempTextureID;
	glGenTextures(1, &tempTextureID);
	glBindTexture(GL_TEXTURE_2D, tempTextureID);

	glTexImage2D(GL_TEXTURE_2D,    // Type of texture
		0,                // Mipmap level (0 being the top level i.e. full size)
		GL_RGBA,          // Internal format
		imageWidth,       // Width of the texture
		imageHeight,      // Height of the texture,
		0,                // Border in pixels
		GL_BGRA,          // Data format
		GL_UNSIGNED_BYTE, // Type of texture data
		textureData);     // The image data to use for this texture

						  // Specify our minification and magnification filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	
	/*
	GLenum glError = glGetError();
	if (glError) {
		std::cout << "There was an error loading the texture: " << filename << std::endl;

		switch (glError) {
			case GL_INVALID_ENUM:
				std::cout << "Invalid enum." << std::endl;
				break;

			case GL_INVALID_VALUE:
				std::cout << "Invalid value." << std::endl;
				break;

			case GL_INVALID_OPERATION:
				std::cout << "Invalid operation." << std::endl;
				break;

			default:
				std::cout << "Unrecognised GLenum." << std::endl;
				break;
		}
	}
	*/

	FreeImage_Unload(bitmap32);

	if (bitsPerPixel != 32) {
		FreeImage_Unload(bitmap);
	}

	return tempTextureID;
}

GLuint getParticleShader(char* vertexShaderSource, char* geometryShaderSource, char* fragmentShaderSource) {
	GLchar *vertexsource, *geometrysource, *fragmentsource;
	GLuint vertexshader, geometryshader, fragmentshader;
	GLuint shaderprogram;
	vertexsource = filetobuf(vertexShaderSource);
	geometrysource = filetobuf(geometryShaderSource);
	fragmentsource = filetobuf(fragmentShaderSource);

	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	glCompileShader(vertexshader);
	GLint IsCompiled_VS;
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);

	if (IsCompiled_VS == GL_FALSE)
	{
		GLint logSize;
		glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(vertexshader, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;
	}

	geometryshader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometryshader, 1, (const GLchar**)&geometrysource, 0);
	glCompileShader(geometryshader);
	GLint IsCompiled_GS;
	glGetShaderiv(geometryshader, GL_COMPILE_STATUS, &IsCompiled_GS);

	if (IsCompiled_GS == GL_FALSE)
	{
		GLint logSize;
		glGetShaderiv(geometryshader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(geometryshader, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;
	}


	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	glCompileShader(fragmentshader);
	GLint IsCompiled_FS;
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
	if (IsCompiled_FS == GL_FALSE)
	{
		GLint logSize;
		glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(fragmentshader, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;
	}


	shaderprogram = glCreateProgram();
	glAttachShader(shaderprogram, vertexshader);
	glAttachShader(shaderprogram, geometryshader);
	glAttachShader(shaderprogram, fragmentshader);

	glLinkProgram(shaderprogram);

	GLint IsLinked;
	glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
	if (IsLinked == GL_FALSE)
	{
		GLint logSize;
		glGetShaderiv(shaderprogram, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(shaderprogram, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;

	}

	return shaderprogram;
}


GLuint getComputeShader(char* computeshadersource) {
	GLchar *computesource;
	GLuint computeshader;
	GLuint shaderprogram;
	computesource = filetobuf(computeshadersource);


	computeshader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeshader, 1, (const GLchar**)&computesource, 0);
	glCompileShader(computeshader);
	GLint IsCompiled_CS;
	glGetShaderiv(computeshader, GL_COMPILE_STATUS, &IsCompiled_CS);

	if (IsCompiled_CS == GL_FALSE) {
		GLint logSize;
		glGetShaderiv(computeshader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(computeshader, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;
	}

	shaderprogram = glCreateProgram();
	glAttachShader(shaderprogram, computeshader);

	glLinkProgram(shaderprogram);

	GLint IsLinked;
	glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
	if (IsLinked == GL_FALSE)
	{
		GLint logSize;
		glGetShaderiv(shaderprogram, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(shaderprogram, logSize, NULL, message);
		std::cout << message << std::endl;
		delete[] message;

	}

	return shaderprogram;
}

char* filetobuf(char *file) {
	FILE *fptr;
	long length;
	char *buf;

	fptr = fopen(file, "rb"); /* Open file for reading */
	if (!fptr) /* Return NULL on failure */
		return NULL;
	fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
	length = ftell(fptr); /* Find out how many bytes into the file we are */
	buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
	fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
	fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
	fclose(fptr); /* Close the file */
	buf[length] = 0; /* Null terminator */

	return buf; /* Return the buffer */
}

void enableBlendMode() {
	glDepthMask(GL_FALSE);
	glBlendEquation(GL_MAX);
}

void disableBlendMode() {
	glDepthMask(GL_TRUE);
	glBlendEquation(GL_FUNC_ADD);
}

glm::vec3 getLookVector(glm::mat4 modelView) {
	return glm::vec3(modelView[0][2], modelView[1][2], modelView[2][2]);
}

glm::vec3 getUpVector(glm::mat4 modelView) {
	return glm::vec3(modelView[0][1], modelView[1][1], modelView[2][1]);
}

glm::vec3 getWorldPosition(glm::mat4 modelView) {
	glm::mat3 rotMat(modelView);
	glm::vec3 d(modelView[3]);

	glm::vec3 retVec = -d * rotMat;
	return retVec;
}
