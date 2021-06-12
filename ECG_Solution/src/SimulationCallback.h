#pragma once

#include <PxPhysicsAPI.h>
#include <iostream>
//#include "Utils.h"

class SimulationCallback : public physx::PxUserControllerHitReport {
private:
	physx::PxRigidActor* _winConditionActor;
	bool* _winCondition;
	bool* _hitDetection;
public:
	SimulationCallback(bool* winCondition, bool* hitDetection)
		: _winCondition(winCondition), _hitDetection(hitDetection) {
	}
	~SimulationCallback() {
	}

	void setWinConditionActor(physx::PxRigidActor* winConditionActor);
	void onShapeHit(const physx::PxControllerShapeHit& hit);
	void onControllerHit(const physx::PxControllersHit& hit);
	void onObstacleHit(const physx::PxControllerObstacleHit& hit);
};