#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
	//std::cout << hit.actor->getName() << std::endl;
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	if (std::strcmp(hit.other->getActor()->getName(), "larry") == 0 && !*dashInProgress) {
		*hitDetection = true;
	}
	else if (std::strcmp(hit.other->getActor()->getName(), "larry") == 0 && *dashInProgress) {
		*enemyDetection = getIntFromCString(hit.controller->getActor()->getName());
	}
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {

}