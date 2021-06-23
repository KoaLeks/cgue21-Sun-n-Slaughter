#pragma once
#include <glm\glm.hpp>
#include <glm\gtc/matrix_transform.hpp>
#include <glm\gtx\euler_angles.hpp>
#include "Shader.h"
#include "Geometry.h"
#include "Node.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <PxPhysicsAPI.h>
#include "PlayerCamera.h"
#include "Enemy.h"
#include "FrustumG.h"
#include "SimulationCallback.h"


class Scene {
protected:
	std::shared_ptr<Shader> _shader;
	std::shared_ptr<Material> _missingMaterial;
	std::shared_ptr<Node> loadScene(string path);
	std::shared_ptr<Node> loadScene(string path, float scale, physx::PxExtendedVec3 position);
	std::string _directory;
	physx::PxPhysics* _physics;
	physx::PxCooking* _cooking;
	physx::PxScene* _scene;
	physx::PxMaterial* _material;
	physx::PxControllerManager* _manager;
	float _angle = 0.0f;
	std::shared_ptr<FrustumG> _viewFrustum;
	unsigned int _drawnObjects;
	irrklang::ISoundEngine* _soundEngine;

	long long* highscore;

public:
	Scene(std::shared_ptr<Shader> shader, char *path, physx::PxPhysics* physics, physx::PxCooking* cooking, physx::PxScene* scene, 
		physx::PxMaterial* material, physx::PxControllerManager* manager, std::shared_ptr<FrustumG> viewFrustum, long long* _highscore, irrklang::ISoundEngine* soundEngine)
		: _shader(shader), _physics(physics), _cooking(cooking), _scene(scene), 
		_material(material), _manager(manager), _viewFrustum(viewFrustum), highscore(_highscore), _soundEngine(soundEngine) {
		_missingMaterial = std::make_shared<TextureMaterial>(_shader, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, "assets/textures/snow.jpg"/*"assets/textures/missing.png"*/);
		_directory = "assets/textures/";
		_drawnObjects = 0;
		loadScene(path);
	}

	void draw();
	void drawDepth(Shader* shader);
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Enemy>> enemies;
	std::shared_ptr<Node> getNodeWithName(std::string name);
	std::shared_ptr<Enemy> getEnemyWithActor(physx::PxRigidActor* actor);

	unsigned int getDrawnObjects() {
		return _drawnObjects;
	}

	void addStaticObject(string path, physx::PxExtendedVec3 position, float scale);
	void addEnemy(physx::PxExtendedVec3 position, float scale, SimulationCallback* simulationCallback);

private:
	std::string floorPrefix = "cook_";
	std::string enemyPrefix = "mob_";
	std::shared_ptr<Node> processNode(aiNode *node, const aiScene *scene, int level, bool transformation, 
		float scale, physx::PxExtendedVec3 position, SimulationCallback* simulationCallback);
	void processMesh(aiMesh *mesh, const aiScene *scene, bool cookMesh, bool isEnemy, std::shared_ptr<Node> newNode, 
		float scale, physx::PxExtendedVec3 position, SimulationCallback* simulationCallback);
	std::shared_ptr<Material> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		string typeName);
};

class Character : public Scene
{
protected:
	glm::vec3 _position;
	glm::vec3 _direction;
	physx::PxController* _pxController;
	PlayerCamera* _camera;
	GLuint pos_vbos[4];
	GLuint norm_vbos[4];
	GLuint pos_out, pos_start, pos_end;
	GLuint norm_out, norm_start, norm_end;
	GLuint uv_pos;
	GLuint index_pos;
	GLuint vector_sizePos;
	GLuint factorPos;
	GLuint _animationShader;
	GLuint vao;
	GLuint vector_size;
	int order[4];
	int hp = 100;
	bool soundIsPlaying = false;

public:
	Character(std::shared_ptr<Shader> shader, char *path, physx::PxPhysics* physics, physx::PxCooking* cooking, 
		physx::PxScene* scene, physx::PxMaterial* material, physx::PxController* c, PlayerCamera* camera, 
		physx::PxControllerManager* manager, GLuint animationShader, std::shared_ptr<FrustumG> viewFrustum, irrklang::ISoundEngine* soundEngine)
		: Scene(shader, path, physics, cooking, scene, material, manager, viewFrustum, nullptr, soundEngine), _pxController(c), 
		_camera(camera), _animationShader(animationShader), order{ 2, 0, 2, 1 } 
	{
		move(0.0f, 0.0f, 0.0f);	
	}
	~Character() {

	}
	void setAngle(float yaw);
	void init();

	void move(float forward, float strafeLeft, float dt);
	void move2(glm::vec3 dir, float speed, float dt);
	void relocate(physx::PxExtendedVec3 pos);
	void updateRotation(float angle);
	glm::vec3 getPosition() {
		return _position;
	}

	int getHP() {
		return hp;
	}

	int inflictDamage(int damage) {

		hp -= damage;

		if (hp < 0) {
			hp = 0;
		}
		else if(!soundIsPlaying){
			soundIsPlaying = true;
			_soundEngine->play2D("assets/audio/mixkit-man-in-pain.mp3", false);
		} else{
			soundIsPlaying = false;
		}
		return hp;
	}

	void animate(int step, int _order[4] = nullptr);

	void setPosition(physx::PxExtendedVec3 pos) {
		_position.x = pos.x;
		_position.y = pos.y;
		_position.z = pos.z;

	}
};