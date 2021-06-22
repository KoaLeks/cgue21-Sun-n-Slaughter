#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
	//std::cout << hit.actor->getName() << std::endl;
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	*_hitDetection = true;
	std::cout << hit.other->getActor()->getName() << std::endl;
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {

}