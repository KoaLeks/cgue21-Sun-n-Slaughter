#pragma once

#include "Node.h"

class Enemy : public Node
{
protected:
	int _hp;
	int _maxHp;
	int _damage;
	glm::vec3 _position;
	physx::PxController* _pxChar;

public:
	Enemy(glm::mat4 modelMatrix = glm::mat4(1.0f));
	~Enemy();
	
	bool hasActor(physx::PxRigidActor* actor);
	bool isDead();
	int hitWithDamage(int damage);
	int getDamage();
	int getHp();
	int getMaxHp();
	void setCharacterController(physx::PxController* pxChar);
	physx::PxController* getCharacterController();
	void move(float forward, float strafeLeft, float dt);
	void updateRotation(float angle);
	void updateCharacter(float dt);
};