#include "PoissonDiskSampling.h"
#include <iostream>

PossionDiskSampling::PossionDiskSampling(int width, int height, float minDist, int count)
{
	this->width = width;
	this->height = height;
	this->minDist = minDist;
	this->count = count;
	srand(time(0));
	generatePossionPoints();
}


PossionDiskSampling::~PossionDiskSampling() {}

std::vector<glm::vec2> PossionDiskSampling::getPoints() {
	return points;
}

std::vector<glm::vec2> PossionDiskSampling::generatePossionPoints() {

	float cellSize = minDist / M_SQRT2;
	//int* grid = new int[ceil(width * height / cellSize)];
	// arr[i][j] is then arr[i * sizeY + j]
	std::vector<std::vector<glm::vec2>> grid(width/cellSize, std::vector<glm::vec2>(height/cellSize, glm::vec2(-1)));
	
	std::vector<glm::vec2> processList;
	std::vector<glm::vec2> spawnPoints;
	//glm::vec2 firstPoint = glm::vec2(rand() % width, rand() % height);
	glm::vec2 firstPoint = glm::vec2(width/2, height/2);
	spawnPoints.push_back(firstPoint);
	processList.push_back(firstPoint);

	glm::vec2 index = imageToGrid(firstPoint, cellSize);
	grid[index.x][index.y] = firstPoint;

	while (processList.size() > 0)
	{
		int randIndex = rand() % processList.size();
		glm::vec2 point = processList[randIndex];
		processList.erase(processList.begin() + randIndex);
		for (int i = 0; i < count; i++)
		{
			glm::vec2 newPoint = generateRandomPointAround(point);
			if (isValid(newPoint) && !inNeighbourhood(grid, newPoint, cellSize)) {
				processList.push_back(newPoint);
				spawnPoints.push_back(newPoint);
				index = imageToGrid(newPoint, cellSize);
				grid[index.x][index.y] = newPoint;
			}
		}
	}

	points = spawnPoints;

	return spawnPoints;
}

glm::vec2 PossionDiskSampling::imageToGrid(glm::vec2 point, float cellSize) {
	int gridX = (point.x / cellSize);
	int gridY = (point.y / cellSize);
	return glm::vec2(std::max(0, gridX-1), std::max(0, gridY-1));
}

glm::vec2 PossionDiskSampling::generateRandomPointAround(glm::vec2 point) {
	float random1 = rand();
	random1 /= RAND_MAX;
	float random2 = rand();
	random2 /= RAND_MAX;
	float radius = minDist * (random1 + 1);
	float angle = 2 * M_PI * random2;

	glm::vec2 newPoint = glm::vec2(
			point.x + radius * cos(angle), 
			point.y + radius * sin(angle));

	return newPoint;
}

bool PossionDiskSampling::isValid(glm::vec2 newPoint) {
	if (newPoint.x >= 0 && newPoint.x < width && newPoint.y >= 0 && newPoint.y < height) {
		return true;
	}
	return false;
}

bool PossionDiskSampling::inNeighbourhood(std::vector<std::vector<glm::vec2>> grid, glm::vec2 newPoint, float cellSize) {
	glm::vec2 index = imageToGrid(newPoint, cellSize);
	std::vector<glm::vec2> neighbours = getNeighbours(grid, index, cellSize);
	for (glm::vec2 cell : neighbours) {
		if (cell.x != -1 && cell.y != -1) {
			if (glm::distance(cell, newPoint) < minDist) {
				return true;
			}
		}
	}
	return false;
}

std::vector<glm::vec2> PossionDiskSampling::getNeighbours(std::vector<std::vector<glm::vec2>> grid, glm::vec2 gridIndex, float cellSize) {
	std::vector<glm::vec2> neighbours;
	int xOffsetStart = std::max(0, (int)gridIndex.x-2);
	int xOffsetEnd = std::min((int)gridIndex.x + 2, (int)grid.size() - 1);
	int yOffsetStart = std::max(0, (int)gridIndex.y - 2);
	int yOffsetEnd = std::min((int)gridIndex.y + 2, (int)grid[0].size() - 1);


	for (int i = xOffsetStart; i < xOffsetEnd; i++)
	{
		for (int j = yOffsetStart; j < yOffsetEnd; j++)
		{
			neighbours.push_back(grid[i][j]);
		}
	}

	return neighbours;
}