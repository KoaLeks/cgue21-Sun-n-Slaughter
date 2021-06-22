#pragma once

#include <PxPhysicsAPI.h>
#include <iostream>
#include "Utils.h"

class SimulationCallback : public physx::PxUserControllerHitReport {
	
private:

	bool* hitDetection;

public:

	SimulationCallback(bool* _hitDetection)
		: hitDetection(_hitDetection) {
	}
	~SimulationCallback() {
	}

	void onShapeHit(const physx::PxControllerShapeHit& hit);
	void onControllerHit(const physx::PxControllersHit& hit);
	void onObstacleHit(const physx::PxControllerObstacleHit& hit);
};