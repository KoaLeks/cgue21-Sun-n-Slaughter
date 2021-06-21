#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
	std::cout << "onShapeHit" << std::endl;
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	std::cout << "onControllerHit" << std::endl;
	*_hitDetection = true;
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {
	std::cout << "onObstacleHit" << std::endl;

}