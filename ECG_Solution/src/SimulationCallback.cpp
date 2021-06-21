#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	*_hitDetection = true;
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {

}