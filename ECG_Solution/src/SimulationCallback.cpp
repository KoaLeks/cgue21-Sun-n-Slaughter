#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
	//std::cout << hit.actor->getName() << std::endl;
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	if (std::strcmp(hit.other->getActor()->getName(), "larry") == 0) {
		*hitDetection = true;

		std::cout << getIntFromCString(hit.controller->getActor()->getName()) << std::endl;
	}
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {

}