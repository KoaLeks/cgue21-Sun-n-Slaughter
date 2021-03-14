#pragma once
#include <GL\glew.h>

class Query {
private:
	GLenum type;
	GLuint id;
	bool inUse;
public:
	Query();
	~Query();
	void init(GLenum type);
	bool isResultReady();
	bool isInUse();
	GLuint getResult();
	void start();
	void end();
};