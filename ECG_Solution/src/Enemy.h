#pragma once

#include "Node.h"
#include "irrklang/irrKlang.h"

class Enemy : public Node
{
protected:
	int hp;
	int maxHp;
	int damage;
	int speed;
	float knockBackDecay;
	float knockBackFactor;
	glm::vec3 _knockBackForce;
	glm::vec3 _position;
	physx::PxExtendedVec3 _spawnPosition;
	physx::PxController* _pxChar;
	irrklang::ISoundEngine* _soundEngine;// = irrklang::createIrrKlangDevice();

	long long* highscore;

public:
	Enemy(long long* _highscore, irrklang::ISoundEngine* soundEngine, glm::mat4 modelMatrix = glm::mat4(1.0f));

	~Enemy();
	
	bool hasActor(physx::PxRigidActor* actor);
	bool isDead(glm::vec3 playerPos);
	void knockBack(glm::vec3 dir, float dt);
	int hitWithDamage(int damage, glm::vec3 dir, float dt, bool hitByDash);
	int getDamage();
	int getHp();
	int getMaxHp();
	void setCharacterController(physx::PxController* pxChar);
	physx::PxController* getCharacterController();
	void move(float forward, float strafeLeft, float dt);
	void updateBoundingBox(glm::vec3 posDelta);
	void move2(glm::vec3 dir, float speed, float dt);
	void updateRotation(float angle);
	void updateCharacter(float dt);
	void chase(glm::vec3& playerPos, float dt);
	void respawn(physx::PxExtendedVec3 position, glm::vec3 playerPos);
	void setSpawnPosition(physx::PxExtendedVec3 position);

};