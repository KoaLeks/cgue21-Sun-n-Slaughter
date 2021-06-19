
#include "Scene.h"

void Scene::draw() {
	_drawnObjects = 0;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->name.compare("cook_map_cook_Plane_Plane")) {
			nodes[i]->draw();
		}
	}
	//std::cout << "Objects: " << _drawnObjects << std::endl << std::endl;
}

void Scene::drawDepth(Shader* shader) {
	_drawnObjects = 0;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->name.compare(0, floorPrefix.size(), floorPrefix)) {
			nodes[i]->drawDepth(shader);
		}
	}
}


std::shared_ptr<Node> Scene::loadScene(string path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return nullptr;
	}
	//directory = path.substr(0, path.find_last_of('/'));
	return processNode(scene->mRootNode, scene, 0, false, 1, physx::PxExtendedVec3(0, 0, 0));
}

std::shared_ptr<Node> Scene::loadScene(string path, float scale, physx::PxExtendedVec3 position) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return nullptr;
	}
	//directory = path.substr(0, path.find_last_of('/'));
	return processNode(scene->mRootNode, scene, 0, true, scale, position);
}

std::shared_ptr<Node> Scene::processNode(aiNode* node, const aiScene* scene, int level, bool transformation, float scale, physx::PxExtendedVec3 position) {
	// process all the node's meshes (if any)
	std::string tmpnam = node->mName.C_Str();
	bool isEnemy = false;
	std::shared_ptr<Node> newNode;

	if (!tmpnam.compare(0, enemyPrefix.size(), enemyPrefix)) {
		isEnemy = true;
		newNode = std::make_shared<Enemy>();
	}
	else {
		newNode = std::make_shared<Node>();
	}

	bool cookMesh = false;
	//bool isWinCondition = false;
	if (!tmpnam.compare(0, floorPrefix.size(), floorPrefix)) {
		cookMesh = true;
		//if (!tmpnam.compare(0, winConditionPrefeix.size(), winConditionPrefeix)) {
		//	isWinCondition = true;
		//}
	}
	newNode->name = tmpnam;

	if (transformation) {
		newNode->setPosition(position);
		newNode->transform(glm::scale(glm::mat4(1), glm::vec3(scale)));
	}

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene, cookMesh, isEnemy/*, isWinCondition*/, newNode, scale, position);
	}

	if (level == 1 && node->mNumMeshes > 0) {
		nodes.push_back(newNode);
		if (isEnemy) {
			enemies.push_back(std::static_pointer_cast<Enemy>(newNode));
		}
	}
	level++;
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		newNode->addChild(processNode(node->mChildren[i], scene, level, transformation, scale, position));
	}

	return newNode;
}


void Scene::processMesh(aiMesh* mesh, const aiScene* scene, bool cookMesh, bool isEnemy/*, bool isWinCondition*/, std::shared_ptr<Node> newNode, float scale, physx::PxExtendedVec3 position) {
	GeometryData data;
	glm::vec3 maxVert(-300.0f, -300.0f, -300.0f);
	glm::vec3 minVert(300.0f, 300.0f, 300.0f);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		glm::vec4 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vector.w = 1.0f;


		// for bounding box
		maxVert.x = std::fmax(maxVert.x, vector.x);
		maxVert.y = std::fmax(maxVert.y, vector.y);
		maxVert.z = std::fmax(maxVert.z, vector.z);
		minVert.x = std::fmin(minVert.x, vector.x);
		minVert.y = std::fmin(minVert.y, vector.y);
		minVert.z = std::fmin(minVert.z, vector.z);


		data.positions.push_back(vector);

		if (mesh->mNormals == nullptr) {
			vector.x = 0;
			vector.y = 1;
			vector.z = 0;
		}
		else {
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
		}

		data.normals.push_back(vector);

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = 1.0f - mesh->mTextureCoords[0][i].y;
			data.uvs.push_back(vec);
		}
		else {
			data.uvs.push_back(glm::vec2(0.0f, 0.0f));
		}
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			data.indices.push_back(face.mIndices[j]);
		}
	}

	std::shared_ptr<Material> mat = _missingMaterial;

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		mat = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	}

	physx::PxRigidActor* meshActor = nullptr;
	physx::PxController* pxChar = nullptr;
	glm::vec3 middlePos = (maxVert + minVert) / 2.0f;
	glm::vec3 lenVec = maxVert - minVert;
	if (cookMesh) {
		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = mesh->mNumVertices;
		meshDesc.points.stride = sizeof(aiVector3D);
		meshDesc.points.data = mesh->mVertices;

		meshDesc.triangles.count = mesh->mNumFaces;
		meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
		meshDesc.triangles.data = &data.indices[0];

		physx::PxDefaultMemoryOutputStream writeBuffer;
		physx::PxTriangleMeshCookingResult::Enum result;
		bool status = _cooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
		if (!status) {
			return;
		}

		physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		physx::PxTriangleMesh* triangleMesh = _physics->createTriangleMesh(readBuffer);

		physx::PxTransform floorPos = physx::PxTransform(/*physx::PxVec3(0.0f, 0.0f, 0.0f)*/position.x, position.y, position.z);
		meshActor = _physics->createRigidStatic(floorPos);
		physx::PxTriangleMeshGeometry geom(triangleMesh, physx::PxMeshScale(scale));
		physx::PxShape* floorShape = physx::PxRigidActorExt::createExclusiveShape(*meshActor, geom, *_material);
		_scene->addActor(*meshActor);
		//if (isWinCondition) {
		//	winConditionActor = meshActor;
		//}
	}
	else if (isEnemy) {
		physx::PxBoxControllerDesc bDesc;
		bDesc.position = physx::PxExtendedVec3(position.x, position.y, position.z);
		bDesc.contactOffset = 0.05f;
		bDesc.halfHeight = lenVec.y / 2.0f;
		bDesc.halfForwardExtent = lenVec.z / 2.0f;
		bDesc.halfSideExtent = lenVec.x / 2.0f;
		bDesc.stepOffset = 0.5f;
		bDesc.slopeLimit = 4.0f;
		bDesc.upDirection = physx::PxVec3(0, 1, 0);
		bDesc.material = _material;
		pxChar = _manager->createController(bDesc);

		meshActor = pxChar->getActor();
		if (isEnemy) {
			std::shared_ptr<Enemy> enemyNode = std::static_pointer_cast<Enemy>(newNode);
			enemyNode->setCharacterController(pxChar);
			enemyNode->setPosition(bDesc.position);
			enemyNode->_startingPosition = -middlePos;
		}
	}
	std::shared_ptr<std::vector<glm::vec3>> boundingBox = std::make_shared<std::vector<glm::vec3>>();
	lenVec = lenVec / 2.0f;
	boundingBox->push_back(middlePos + glm::vec3(lenVec.x, lenVec.y, lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(lenVec.x, lenVec.y, -lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(lenVec.x, -lenVec.y, lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(lenVec.x, -lenVec.y, -lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(-lenVec.x, lenVec.y, lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(-lenVec.x, lenVec.y, -lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(-lenVec.x, -lenVec.y, lenVec.z));
	boundingBox->push_back(middlePos + glm::vec3(-lenVec.x, -lenVec.y, -lenVec.z));

	newNode->addMesh(std::make_shared<Geometry>(glm::mat4(1.0f), data, mat, meshActor, pxChar, boundingBox, _viewFrustum, &_drawnObjects));
}


std::shared_ptr<Material> Scene::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
	std::shared_ptr<Material> materialTexture = _missingMaterial;
	aiColor3D ambientColor(0.f, 0.f, 0.f);
	mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
	aiColor3D difuseColor(0.f, 0.f, 0.f);
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, difuseColor);
	aiColor3D specularColor(0.f, 0.f, 0.f);
	mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
	float specularExp = 1.0f;
	mat->Get(AI_MATKEY_SHININESS, specularExp);

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string pathToTextrue = _directory + str.C_Str();
		// full color support maybe later
		materialTexture = std::make_shared<TextureMaterial>(_shader, glm::vec3(ambientColor.r, difuseColor.r, specularColor.r), 1.0f, pathToTextrue.c_str());
	}
	return materialTexture;
}

void Character::move(float forward, float strafeLeft, float dt) {
	float addX = forward * glm::cos(glm::radians(_angle)) + strafeLeft * glm::sin(-glm::radians(_angle));
	float addY = forward * glm::sin(glm::radians(_angle)) + strafeLeft * glm::cos(-glm::radians(_angle));


	_pxController->move(physx::PxVec3(addY, -10.0f, addX), 0.001f, dt, physx::PxControllerFilters());
	//std::cout << "x: " << _pxController->getPosition().x << ", y: " << _pxController->getPosition().y << ", z: " << _pxController->getPosition().z << std::endl;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		nodes[i]->setPosition(_pxController->getPosition());
	}
	setPosition(_pxController->getPosition());
	_camera->setPosition(_pxController->getPosition());
}

void Character::relocate(physx::PxExtendedVec3 pos) {
	_pxController->setPosition(pos);
	//std::cout << "x: " << _pxController->getPosition().x << ", y: " << _pxController->getPosition().y << ", z: " << _pxController->getPosition().z << std::endl;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		nodes[i]->setPosition(_pxController->getPosition());
	}
	setPosition(_pxController->getPosition());
	_camera->setPosition(_pxController->getPosition());
}

void Character::updateRotation(float angle) {
	_angle = angle;
	for (unsigned int i = 0; i < nodes.size(); i++) {
		nodes[i]->yaw(angle);
	}
}

std::shared_ptr<Node> Scene::getNodeWithName(std::string name) {
	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->name == name) {
			return nodes[i];
		}
	}
	return nullptr;
}

void Character::setAngle(float yaw) {
	_angle = yaw;
}

std::shared_ptr<Enemy> Scene::getEnemyWithActor(physx::PxRigidActor* actor) {
	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i]->hasActor(actor)) {
			return enemies[i];
		}
	}
	return nullptr;
}

void Character::init() {
	glUseProgram(_animationShader);
	vector_size = nodes[0]->_meshes[0]->vector_size;
	glGenBuffers(1, &pos_start);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos_start);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &norm_start);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, norm_start);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &pos_end);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos_end);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &norm_end);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, norm_end);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &pos_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &norm_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, norm_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vector_size * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	vector_sizePos = glGetUniformLocation(_animationShader, "vector_size");
	factorPos = glGetUniformLocation(_animationShader, "inter");

	_shader->use();

	uv_pos = nodes[0]->_meshes[0]->_vboUVs;
	index_pos = nodes[0]->_meshes[0]->_vboIndices;
	nodes[0]->_meshes[0]->_isCharacter = true;

	for (int i = 0; i < 3; i++) {
		pos_vbos[i] = nodes[i]->_meshes[0]->_vboPositions;
		norm_vbos[i] = nodes[i]->_meshes[0]->_vboNormals;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, pos_out);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, norm_out);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uv_pos);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_pos);

	glBindVertexArray(0);
	_shader->unuse();
}


void Character::animate(int step) {
	glUseProgram(_animationShader);

	glUniform1ui(vector_sizePos, vector_size);
	glUniform1f(factorPos, float(step % 15) / 15.0f);

	int input = order[(step / 15) % 4];
	int output = order[((step / 15) + 1) % 4];

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pos_vbos[input]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, norm_vbos[input]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pos_vbos[output]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, norm_vbos[output]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pos_out);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, norm_out);

	GLuint groups = (vector_size / (32 * 32) + 1);
	glDispatchCompute(groups, 1, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
	nodes[0]->_meshes[0]->_vao = vao;
	nodes[0]->draw();
}

//physx::PxRigidActor* Scene::getWinConditionActor() {
//	return winConditionActor;
//}

void Scene::addStaticObject(string path, physx::PxExtendedVec3 position, float scale)
{
	std::shared_ptr<Node> newNode = loadScene(path, scale, position);
	//nodes[nodes.size() - 1]->setPosition(position);
	//nodes[nodes.size() - 1]->transform(glm::scale(glm::mat4(1), glm::vec3(scale)));

}

void Scene::addEnemy(physx::PxExtendedVec3 position, float scale) {
	//if (enemyMaster == nullptr) {
		Assimp::Importer import;
		const aiScene* enemyMasterX = import.ReadFile("assets/models/Cubex_notMob.obj", aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!enemyMasterX || enemyMasterX->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !enemyMasterX->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
//	}

	processNode(enemyMasterX->mRootNode, enemyMasterX, 0, true, scale, position);
}
