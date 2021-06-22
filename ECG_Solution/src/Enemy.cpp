
#include "Enemy.h"

Enemy::Enemy(glm::mat4 modelMatrix) : Node(modelMatrix) {
	_hp = 50;
	_maxHp = 50;
	_damage = 5;
	_angle = 0;
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

bool Enemy::isDead() {
	if (_hp <= 0) {
		_enabled = false;
		respawn(_spawnPosition, 1.0f);
		return true;
	} else {
		return false;
	}
}

int Enemy::hitWithDamage(int damage) {
	_hp -= damage;
	isDead();
	return _hp;
}

int Enemy::getDamage() {
	return _damage;
}

int Enemy::getHp() {
	return _hp;
}

int Enemy::getMaxHp() {
	return _maxHp;
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
	float addX = speed * dir.x; 
	float addZ = speed * dir.z;
	glm::vec3 oldPos = this->getPosition();

	_pxChar->move(physx::PxVec3(addX, -98.0f, addZ) * dt, 0.001f, dt, physx::PxControllerFilters());
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

void Enemy::chase(glm::vec3& playerPos, float speed, float dt) {
	glm::vec3 enemyPos = glm::vec3(_pxChar->getPosition().x, _pxChar->getPosition().y, _pxChar->getPosition().z);
	glm::vec3 direction = glm::normalize(playerPos - enemyPos);
	float angle = std::atan2(-direction.z, direction.x);;
	
	this->updateRotation(glm::degrees(angle));
	//this->move(2 * -direction.x, 2 * direction.z, dt);
	this->move2(direction, speed, dt);
}

void Enemy::respawn(physx::PxExtendedVec3 position, float scale)
{

	glm::vec3 oldPos = this->getPosition();
	_pxChar->setPosition(position);
	//node->setPosition(_pxChar->getPosition());
	setPosition(_pxChar->getPosition());
	_enabled = true;
	glm::vec3 currentPos = getPosition();
	updateBoundingBox(currentPos - oldPos);
	//More modifications?
	_hp = _maxHp;
}

void Enemy::setSpawnPosition(physx::PxExtendedVec3 position)
{
	_spawnPosition = position;
}
