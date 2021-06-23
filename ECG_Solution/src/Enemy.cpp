
#include "Enemy.h"
#include <assimp\color4.h>

Enemy::Enemy(long long* _highscore, irrklang::ISoundEngine* soundEngine, glm::mat4 modelMatrix) : Node(modelMatrix), highscore(_highscore) {
	hp = 50;
	maxHp = 50;
	damage = 5;
	speed = 20;
	knockBackFactor = 3;
	_angle = 0;
	_knockBackForce = glm::vec3(0); 
	_soundEngine = soundEngine;
	knockBackDecay = 1;
}

Enemy::~Enemy() {
}

bool Enemy::hasActor(physx::PxRigidActor* actor) {
	for (size_t i = 0; i < _meshes.size(); i++) {
		if (_meshes[i]->getActor() == actor) {
			return true;
		}
	}

	return false;
}

bool Enemy::isDead(glm::vec3 playerPos) {
	if (hp <= 0) {
		_soundEngine->play2D("assets/audio/mixkit-mythical-beast-growl.wav", false);
		//Add highscore
		*highscore += ((100 * damage) - 400);

		_enabled = false;
		respawn(_spawnPosition, playerPos);
		return true;
	} else {
		return false;
	}
}

void Enemy::knockBack(glm::vec3 dir, float dt) {
	float addX = knockBackFactor * speed * dir.x;
	float addZ = knockBackFactor * speed * dir.z;
	_knockBackForce = glm::vec3(addX, 0, addZ);
}


int Enemy::hitWithDamage(int damage, glm::vec3 dir, float dt, bool hitByDash) {
	hp -= damage;
	if (hitByDash) {
		knockBackFactor = 10;
		knockBack(dir, dt);
	} else {
		knockBackFactor = 3;
		knockBack(dir, dt);
	}
	return hp;
}

int Enemy::getDamage() {
	return damage;
}

int Enemy::getHp() {
	return hp;
}

int Enemy::getMaxHp() {
	return maxHp;
}

void Enemy::setCharacterController(physx::PxController* pxChar) {
	_pxChar = pxChar;
}

physx::PxController* Enemy::getCharacterController() {
	return _pxChar;
}

void Enemy::move(float forward, float strafeLeft, float dt) {
	float addX = forward * glm::sin(glm::radians(_angle)) + strafeLeft * glm::cos(-glm::radians(_angle));
	float addY = forward * glm::cos(glm::radians(_angle)) + strafeLeft * glm::sin(-glm::radians(_angle));

	_pxChar->move(physx::PxVec3(addX, 0.0f, addY), 0.001f, dt, physx::PxControllerFilters());

	this->setPosition(_pxChar->getPosition());

}

void Enemy::updateBoundingBox(glm::vec3 posDelta) {
	for (int i = 0; i < this->_meshes.size(); i++) {
		this->_meshes[i]->updateBoundingBox(posDelta);
	}
}

void Enemy::move2(glm::vec3 dir, float speed, float dt) {
	float addX = speed * dir.x + _knockBackForce.x;
	float addZ = speed * dir.z + _knockBackForce.z;
	glm::vec3 oldPos = this->getPosition();

	if (oldPos.x + addX > 1000) {
		addX = 1000 - oldPos.x;
	}
	else if (oldPos.x + addX < 10) {
		addX = 10 - oldPos.x;
	}
	if (oldPos.z + addZ < -1000) {
		addZ = -1000 - oldPos.z;
	}
	else if (oldPos.z + addZ > -10) {
		addZ = -10 - oldPos.z;
	}

	_knockBackForce = _knockBackForce + (knockBackDecay * dt) * (glm::vec3(0) - _knockBackForce);

	_pxChar->move(physx::PxVec3(addX , -98.0f, addZ ) * dt, 0.001f, dt, physx::PxControllerFilters());
	//_pxChar->getActor()->addForce(physx::PxVec3(addX, -0.0f, addZ));
	this->setPosition(_pxChar->getPosition());
	glm::vec3 currentPos = this->getPosition();
	updateBoundingBox(currentPos - oldPos);
}

void Enemy::updateRotation(float value) {
	_angle = int(90 + value) % 360;
	this->yaw(_angle);
	
}

void Enemy::updateCharacter(float dt) {
	this->updateRotation(30.0f * dt);
	this->move(1.8f * dt, 0.0f, dt);
}

void Enemy::chase(glm::vec3& playerPos, float dt) {
	glm::vec3 enemyPos = glm::vec3(_pxChar->getPosition().x, _pxChar->getPosition().y, _pxChar->getPosition().z);
	glm::vec3 direction = glm::normalize(playerPos - enemyPos);
	float angle = std::atan2(-direction.z, direction.x);;
	
	this->updateRotation(glm::degrees(angle));
	//this->move(2 * -direction.x, 2 * direction.z, dt);
	this->move2(direction, speed, dt);
}

void Enemy::respawn(physx::PxExtendedVec3 position, glm::vec3 playerPos)
{
	_knockBackForce = glm::vec3(0);
	glm::vec3 oldPos = this->getPosition();
	if (glm::distance(glm::vec3(position.x, 0, position.z), glm::vec3(playerPos.x, 0, playerPos.z)) < 50) {
		_pxChar->setPosition(physx::PxExtendedVec3(position.x - 70, position.y + 15, position.z - 70));
	}
	else {
		_pxChar->setPosition(position);
	}
	//node->setPosition(_pxChar->getPosition());
	setPosition(_pxChar->getPosition());
	_enabled = true;
	glm::vec3 currentPos = getPosition();
	updateBoundingBox(currentPos - oldPos);

	//Modifications
	maxHp += 5;
	damage++;
	speed += 0.2f;

	hp = maxHp;
}

void Enemy::setSpawnPosition(physx::PxExtendedVec3 position)
{
	_spawnPosition = position;
}
