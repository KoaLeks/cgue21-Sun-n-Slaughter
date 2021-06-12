#include "SimulationCallback.h"

void SimulationCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
	if (hit.actor == _winConditionActor) {
		*_winCondition = true;
	}
}

void SimulationCallback::onControllerHit(const physx::PxControllersHit& hit) {
	*_hitDetection = true;
}

void SimulationCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {

}


void SimulationCallback::setWinConditionActor(physx::PxRigidActor* winConditionActor) {
	_winConditionActor = winConditionActor;
}