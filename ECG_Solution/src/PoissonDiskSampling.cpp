#include "PoissonDiskSampling.h"
#include <iostream>
#include "stb_image.h"

PossionDiskSampling::PossionDiskSampling(int terrainSize, const char* maskPath, const char* heightMapPath, float scaleY, float minDist, int count)
{
	this->width = terrainSize;
	this->height = terrainSize;
	this->minDist = minDist;
	this->count = count;
	srand(time(0));
	generatePossionPoints();
	applyMask(maskPath);
	applyTerrainHeight(heightMapPath, scaleY);
}

PossionDiskSampling::~PossionDiskSampling() {}

std::vector<glm::vec3> PossionDiskSampling::getPoints() {
	return points3D;
}

std::vector<glm::vec2> PossionDiskSampling::applyMask(const char* maskPath) {
	int imgWidth, imgHeight, nrChannels;
	unsigned char* data = stbi_load(maskPath, &imgWidth, &imgHeight, &nrChannels, 4);
	for (auto p = points2D.begin(); p != points2D.end();) {
		glm::vec2 pixelPos = glm::vec2(floor(p->x / this->width * imgWidth),floor(p->y / this->height * imgHeight));
		unsigned char* value = data + 4 * int((pixelPos.y * imgWidth + pixelPos.x));
		unsigned char r = value[0];
		if (r == 0) {
			p = points2D.erase(p);
		}
		else
		{
			++p;
		}
	}
	return points2D;
}

std::vector<glm::vec3>  PossionDiskSampling::applyTerrainHeight(const char* heightMapPath, float scaleY) {
	int imgWidth, imgHeight, nrChannels;
	unsigned char* data = stbi_load(heightMapPath, &imgWidth, &imgHeight, &nrChannels, 4);

	for (auto p = points2D.begin(); p != points2D.end();) {
		glm::vec2 pixelPos = glm::vec2(floor(p->x / this->width * imgWidth), floor(p->y / this->height * imgHeight));
		unsigned char* value = data + 4 * int((pixelPos.y * imgWidth + pixelPos.x));
		float redChannel = float(value[0]) / 255;
		float height = redChannel * scaleY;
		glm::vec3 pos3D = glm::vec3(p->x, height, p->y);
		points3D.push_back(pos3D);
		++p;
	}
	return points3D;
}

std::vector<glm::vec2> PossionDiskSampling::generatePossionPoints() {
	float cellSize = minDist / M_SQRT2;
	std::vector<std::vector<glm::vec2>> grid(width/cellSize, std::vector<glm::vec2>(height/cellSize, glm::vec2(-1)));
	
	std::vector<glm::vec2> processList;
	std::vector<glm::vec2> spawnPoints;
	//glm::vec2 firstPoint = glm::vec2(rand() % width, rand() % height);
	glm::vec2 firstPoint = glm::vec2(512, 512);
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
	points2D = spawnPoints;
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