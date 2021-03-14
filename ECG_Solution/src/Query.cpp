#include "Query.h"
#include <iostream>

Query::Query() {
	this->inUse = false;
}

Query::~Query() {
	glDeleteQueries(1, &id);
}

void Query::init(GLenum type){
	this->type = type;
	glGenQueries(1, &id);
	glBeginQuery(type, id);
	glEndQuery(type);
}

void Query::start() {
	glBeginQuery(type, id);
	inUse = true;
}

void Query::end() {
	glEndQuery(type);
	inUse = false;
}

GLuint Query::getResult() {
	GLuint params;
	glGetQueryObjectuiv(id, GL_QUERY_RESULT, &params);
	return params;
}

bool Query::isResultReady() {
	GLint params;
	glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &params);
	return params == GL_TRUE;
}

bool Query::isInUse() {
	return inUse;
}
