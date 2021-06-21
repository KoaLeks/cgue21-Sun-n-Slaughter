#pragma once

#include <PxPhysicsAPI.h>
#include <iostream>
#include "Utils.h"

class SimulationCallback : public physx::PxUserControllerHitReport {
	
private:

	bool* _hitDetection;

public:

	SimulationCallback(bool* hitDetection)
		: _hitDetection(hitDetection) {
	}
	~SimulationCallback() {
	}

	void onShapeHit(const physx::PxControllerShapeHit& hit);
	void onControllerHit(const physx::PxControllersHit& hit);
	void onObstacleHit(const physx::PxControllerObstacleHit& hit);
};