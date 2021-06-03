#pragma once
#include <vector>
#include <glm\detail\type_vec.hpp>
#include <glm\glm.hpp>
#include <random>
#include <ctime>


#define _USE_MATH_DEFINES
#include "math.h"

class PossionDiskSampling {

public:
	PossionDiskSampling(int width, int height, float minDist, int count);
	~PossionDiskSampling();
	std::vector<glm::vec2> getPoints();
	void applyMask(const char* maskPath);

private: 
	int width;
	int height;
	float minDist;
	int count;
	std::vector<glm::vec2> points;

	std::vector<glm::vec2> generatePossionPoints();
	glm::vec2 imageToGrid(glm::vec2 point, float cellSize);
	glm::vec2 generateRandomPointAround(glm::vec2 point);
	bool isValid(glm::vec2 newPoint);
	bool inNeighbourhood(std::vector<std::vector<glm::vec2>> grid, glm::vec2 newPoint, float cellSize);
	std::vector<glm::vec2> getNeighbours(std::vector<std::vector<glm::vec2>> grid, glm::vec2 gridIndex, float cellSize);
};