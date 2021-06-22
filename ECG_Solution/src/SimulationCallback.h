#pragma once

#include <PxPhysicsAPI.h>
#include <iostream>
#include "Utils.h"

class SimulationCallback : public physx::PxUserControllerHitReport {
	
private:

	bool* hitDetection;
	int* enemyDetection;
	bool* dashInProgress;

public:

	SimulationCallback(bool* _hitDetection, int* _enemyDetection, bool* _dashInProgress)
		: hitDetection(_hitDetection), enemyDetection(_enemyDetection), dashInProgress(_dashInProgress) {
	}
	~SimulationCallback() {
	}

	void onShapeHit(const physx::PxControllerShapeHit& hit);
	void onControllerHit(const physx::PxControllersHit& hit);
	void onObstacleHit(const physx::PxControllerObstacleHit& hit);
};