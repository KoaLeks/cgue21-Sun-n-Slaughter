/*
* Copyright 2019 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/


#include "Utils.h"
#include <sstream>
#include "Camera.h"
#include "Shader.h"
#include "Geometry.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include "Mesh.h"
#include "Terrain/TerrainShader.h"
#include "Terrain/Terrain.h"
#include "Skybox/Skybox.h"
#include "Shadowmap/ShadowMap.h"
#include "GUI/GuiTexture.h"
#include "GUI/GuiRenderer.h"
#include "Flare/FlareManager.h"
#include "PoissonDiskSampling.h"

/* GAMEPLAY */
#include <PxPhysicsAPI.h>
#include <FreeImagePlus.h>
#include "SimulationCallback.h"
#include "PlayerCamera.h"
#include "Scene.h"
#include "FrustumG.h"
using namespace physx;
/* GAMEPLAY END */


/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
/* GAMEPLAY */
bool move_character(GLFWwindow* window, Character* character, float deltaMovement);
/* GAMEPLAY END */
void setPerFrameUniformsNormal(Shader* shader, PlayerCamera& camera, PointLight& pointL);
void setPerFrameUniforms(TerrainShader* shader, PlayerCamera& camera, PointLight& pointL);
float getYPosition(float x, float z);
void renderQuad();


/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

//static bool _wireframe = false;
//static bool _culling = true;
//static bool _dragging = false;
static bool _strafing = false;
//static float _zoom = 5.0f;
static int sreen_width = 1600;
static int sreen_height= 900;
static char* heightMapPath = "assets/terrain/hm3.png";
static char* treeMaskPath = "assets/terrain/mask1.png";

/* GAMEPLAY */
static bool _wireframe = false;
static bool _culling = true;
static bool _dragging = true; //= false;
static bool _draggingCamOnly = false;
static bool _mouseSelect = false;
static bool _doBasicAttack = false;
static bool _doStrongAttack = false;
static bool _doAreacAttack = false;
static bool _checkFrustum = true;
static bool _showHelp = false;
static float _zoom = -6.0f;
double lastxpos = 0;
double lastypos = 0;
int _selectedFPS = 60;
static bool _limitFPS = true;
bool _winCondition = false;
bool _hitDetection = false;
float brightness = 1.0;
int imgWidth, imgHeight, nrChannels;
unsigned char* data;

int terrainPlaneSize = 1024;
int terrainHeight = 250;
float lightDistance = 20000.0f;
/* GAMEPLAY END */

/* --------------------------------------------- */
// Main
/* --------------------------------------------- */

int main(int argc, char** argv)
{
	/* --------------------------------------------- */
	// Load settings.ini
	/* --------------------------------------------- */

	INIReader reader("assets/settings.ini");

	int window_width = reader.GetInteger("window", "width", 1600);
	int window_height = reader.GetInteger("window", "height", 900);
	int refresh_rate = reader.GetInteger("window", "refresh_rate", 60);
	bool fullscreen = reader.GetBoolean("window", "fullscreen", false);
	std::string window_title = reader.Get("window", "title", "Sun'n'Slaughter");
	float fov = float(reader.GetReal("camera", "fov", 60.0f));
	float nearZ = float(reader.GetReal("camera", "near", 0.1f));
	float farZ = float(reader.GetReal("camera", "far", 100000.0f));
	brightness = float(reader.GetReal("window", "brightness", 1.0));

	/* GAMEPLAY */
	_selectedFPS = reader.GetInteger("window", "fps", 60);
	/* GAMEPLAY END */

	/* --------------------------------------------- */
	// Create context
	/* --------------------------------------------- */

	glfwSetErrorCallback([](int error, const char* description) { std::cout << "GLFW error " << error << ": " << description << std::endl; });

	if (!glfwInit()) {
		EXIT_WITH_ERROR("Failed to init GLFW");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Request OpenGL version 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Request core profile
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // Create an OpenGL debug context 
	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate); // Set refresh rate
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Enable antialiasing (4xMSAA)
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Open window
	GLFWmonitor* monitor = nullptr;

	if (fullscreen)
		monitor = glfwGetPrimaryMonitor();

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title.c_str(), monitor, nullptr);

	if (!window) {
		EXIT_WITH_ERROR("Failed to create window");
	}

	/* GAMEPLAY */
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	/* GAMEPLAY END */

	// This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	GLenum err = glewInit();

	// If GLEW wasn't initialized
	if (err != GLEW_OK) {
		EXIT_WITH_ERROR("Failed to init GLEW: " << glewGetErrorString(err));
	}

	// Debug callback
	if (glDebugMessageCallback != NULL) {
		// Register your callback function.

		glDebugMessageCallback(DebugCallbackDefault, NULL);
		// Enable synchronous callback. This ensures that your callback function is called
		// right after an error has occurred. This capability is not defined in the AMD
		// version.
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	/* GAMEPLAY */
	// for HUD overlay
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/* GAMEPLAY END */

	/* GAMEPLAY */

	/* --------------------------------------------- */
	// Init Physx
	/* --------------------------------------------- */

	PxDefaultErrorCallback gDefaultErrorCallback;
	PxDefaultAllocator gDefaultAllocatorCallback;
	PxFoundation* gFoundation = nullptr;
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	
	PxPvd*  pvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10000);
	pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	
	PxPhysics* gPhysicsSDK = nullptr;
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, pvd);
	if (gPhysicsSDK == nullptr) {
		EXIT_WITH_ERROR("Failed to init physx")
	}

	PxCooking* gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	if (!gCooking) {
		EXIT_WITH_ERROR("Failed to init cooking")
	}

	SimulationCallback* simulatonCallback = new SimulationCallback(&_winCondition, &_hitDetection);
	PxScene* gScene = nullptr;
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	// sceneDesc.simulationEventCallback = simulatonCallback;
	gScene = gPhysicsSDK->createScene(sceneDesc);
	PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.5f);

	PxControllerManager* gManager = PxCreateControllerManager(*gScene);

	PxCapsuleControllerDesc cDesc;
	cDesc.position = PxExtendedVec3(1024 / 2, 100.0f, -1024 / 2);
	cDesc.contactOffset = 0.05f;
	cDesc.height = 2.0f;
	cDesc.radius = 1.0f;
	cDesc.stepOffset = 0.2f;
	cDesc.slopeLimit = 0.2f;
	cDesc.upDirection = PxVec3(0, 1, 0);
	cDesc.material = mMaterial;
	cDesc.reportCallback = simulatonCallback;
	PxController* pxChar = gManager->createController(cDesc);

	/* GAMEPLAY END */


	/* --------------------------------------------- */
	// Init framework
	/* --------------------------------------------- */

	if (!initFramework()) {
		EXIT_WITH_ERROR("Failed to init framework");
	}

	/* GAMEPLAY */
	FreeImage_Initialise(true);
	/* GAMEPLAY END */

	// set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	/* GAMEPLAY */
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	/* GAMEPLAY END */

	// set GL defaults
	glClearColor(1, 1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	/* GAMEPLAY */

	/* --------------------------------------------- */
	// Init HUD / Text
	/* --------------------------------------------- */

	//TextRenderer* text = new TextRenderer(window_width, window_height);
	//text->Load("assets/fonts/MetalMania-Regular.ttf", 32);
	//GLuint frameHud = loadTextureFromFile("assets/textures/frame_hud.png");

	/* GAMEPLAY END */


	/* --------------------------------------------- */
	// Initialize scene and render loop
	/* --------------------------------------------- */
	{

		// Load shader(s)
		std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("texture.vert", "texture.frag");
		std::shared_ptr<Shader> skyboxShader = std::make_shared<Shader>("skybox.vert", "skybox.frag");
		std::shared_ptr<Shader> shadowMapDebugShader = std::make_shared<Shader>("shadowMapQuadDebug.vert", "shadowMapQuadDebug.frag");
		std::shared_ptr<Shader> shadowMapDepthShader = std::make_shared<Shader>("shadowmap_depth.vert", "shadowmap_depth.frag");
		std::shared_ptr<Shader> guiShader = std::make_shared<Shader>("gui.vert", "gui.frag");
		std::shared_ptr<TerrainShader> tessellationShader = std::make_shared<TerrainShader>(
			"assets/shader/terrain.vert", 
			"assets/shader/terrain.tessc", 
			"assets/shader/terrain.tesse", 
			"assets/shader/terrain.frag"
			);


		// Create Terrain
		// heightmap muss ein vielfaches von 20 (oder 2^n?) sein, ansonsten wirds nicht korrekt abgebildet
		Terrain plane = Terrain(terrainPlaneSize, 50, terrainHeight, heightMapPath, false);
		Terrain planeShadow = Terrain(terrainPlaneSize, 50, terrainHeight, heightMapPath, true);

		// Create Skybox
		Skybox skybox = Skybox(skyboxShader.get());

		// Initialize camera
		//Camera camera(fov, float(window_width) / float(window_height), nearZ, farZ);
		//camera.update(window_width, window_height, false, false, false);

		// Initialize lights
		//PointLight pointL(glm::vec3(.5f), glm::vec3(50000, lightDistance, 0), glm::vec3(0.08f, 0.03f, 0.01f));
		PointLight pointL(glm::vec3(.5f), glm::vec3(-1500, 1700, -2500), glm::vec3(0.08f, 0.03f, 0.01f));

		// Shadow Map
		ShadowMap shadowMap = ShadowMap(shadowMapDepthShader.get(), pointL.position, nearZ/10, farZ/100, 1250.0f, glm::vec3(terrainPlaneSize/2, 0, -terrainPlaneSize/2));

		std::shared_ptr<MeshMaterial> material = std::make_shared<MeshMaterial>(textureShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);
		std::shared_ptr<MeshMaterial> depth = std::make_shared<MeshMaterial>(shadowMapDepthShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);		

		// Tree positions
		PossionDiskSampling treePositions = PossionDiskSampling(terrainPlaneSize, treeMaskPath, heightMapPath, terrainHeight, 80, 20);
		std::vector<glm::vec3> points = treePositions.getPoints();

		// GUI
		std::vector<GuiTexture> guis;
		Texture heart = Texture("assets/heart.png", true);
		float scaleFactor = 50.0f;
		glm::vec2 scale = glm::vec2(scaleFactor * heart.getAspectRatio() / window_width, scaleFactor / window_height);
		GuiTexture gui1 = GuiTexture(heart.getTextureId(), glm::vec2(-0.95f, 0.9f), scale);
		GuiTexture gui2 = GuiTexture(heart.getTextureId(), glm::vec2(-0.88f, 0.9f), scale);
		GuiTexture gui3 = GuiTexture(heart.getTextureId(), glm::vec2(-0.81f, 0.9f), scale);
		GuiTexture gui4 = GuiTexture(heart.getTextureId(), glm::vec2(-0.74f, 0.9f), scale);
		guis.push_back(gui1);
		guis.push_back(gui2);
		guis.push_back(gui3);
		guis.push_back(gui4);
		GuiRenderer guiRenderer = GuiRenderer(guiShader.get());

		// Flare
		std::vector<GuiTexture> flares;
		Texture flare1 = Texture("assets/flares/tex1.png", true);
		Texture flare2 = Texture("assets/flares/tex2.png", true);
		Texture flare3 = Texture("assets/flares/tex3.png", true);
		Texture flare4 = Texture("assets/flares/tex4.png", true);
		Texture flare5 = Texture("assets/flares/tex5.png", true);
		Texture flare6 = Texture("assets/flares/tex6.png", true);
		Texture flare7 = Texture("assets/flares/tex7.png", true);
		Texture flare8 = Texture("assets/flares/tex8.png", true);
		Texture flare9 = Texture("assets/flares/tex9.png", true);
		
		GuiTexture flareGui1 = GuiTexture(flare1.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui2 = GuiTexture(flare2.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui3 = GuiTexture(flare3.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui4 = GuiTexture(flare4.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui5 = GuiTexture(flare5.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui6 = GuiTexture(flare6.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui7 = GuiTexture(flare7.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui8 = GuiTexture(flare8.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
		GuiTexture flareGui9 = GuiTexture(flare9.getTextureId(), glm::vec2(0.0f), glm::vec2(1.0f));
			
		flares.push_back(GuiTexture(flare6.getTextureId(), glm::vec2(0.0f), glm::vec2(0.5f)));
		flares.push_back(GuiTexture(flare4.getTextureId(), glm::vec2(0.0f), glm::vec2(0.23f)));
		flares.push_back(GuiTexture(flare2.getTextureId(), glm::vec2(0.0f), glm::vec2(0.1f)));
		flares.push_back(GuiTexture(flare7.getTextureId(), glm::vec2(0.0f), glm::vec2(0.05f)));
		flares.push_back(GuiTexture(flare1.getTextureId(), glm::vec2(0.0f), glm::vec2(0.02f)));
		flares.push_back(GuiTexture(flare3.getTextureId(), glm::vec2(0.0f), glm::vec2(0.06f)));
		flares.push_back(GuiTexture(flare9.getTextureId(), glm::vec2(0.0f), glm::vec2(0.012f)));
		flares.push_back(GuiTexture(flare5.getTextureId(), glm::vec2(0.0f), glm::vec2(0.07f)));
		flares.push_back(GuiTexture(flare1.getTextureId(), glm::vec2(0.0f), glm::vec2(0.012f)));
		flares.push_back(GuiTexture(flare7.getTextureId(), glm::vec2(0.0f), glm::vec2(0.02f)));
		flares.push_back(GuiTexture(flare9.getTextureId(), glm::vec2(0.0f), glm::vec2(0.1f)));
		flares.push_back(GuiTexture(flare3.getTextureId(), glm::vec2(0.0f), glm::vec2(0.07f)));
		flares.push_back(GuiTexture(flare5.getTextureId(), glm::vec2(0.0f), glm::vec2(0.3f)));
		flares.push_back(GuiTexture(flare4.getTextureId(), glm::vec2(0.0f), glm::vec2(0.4f)));
		flares.push_back(GuiTexture(flare8.getTextureId(), glm::vec2(0.0f), glm::vec2(0.6f)));

		FlareManager flareMangaer = FlareManager(guiShader.get(), 0.15f, flares);

		/* GAMEPLAY */
		/* --------------------------------------------- */
		// Init Light mapping
		/* --------------------------------------------- */

		// Initialize camera
		PlayerCamera playerCamera(fov, float(window_width) / float(window_height), nearZ, farZ);
		std::shared_ptr<FrustumG> viewFrustum = std::make_shared<FrustumG>();
		viewFrustum->setCamInternals(fov, float(window_width) / float(window_height), nearZ, farZ);
		glm::mat4 camModel = playerCamera.getModel();
		viewFrustum->setCamDef(getWorldPosition(camModel), getLookVector(camModel), getUpVector(camModel));

		//std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("texture.vert", "texture.frag");
		Scene level(textureShader, "assets/models/cook_map_detailed.obj", gPhysicsSDK, gCooking, gScene, mMaterial, gManager, viewFrustum);
		simulatonCallback->setWinConditionActor(level.getWinConditionActor());

		// Load heightmap
		data = stbi_load(heightMapPath, &imgWidth, &imgHeight, &nrChannels, 4);
		
		// Load trees
		for (glm::vec3 pos : points)
		{
			pos.z -= terrainPlaneSize;
			level.addStaticObject("assets/models/trees/cook_baum.obj", PxExtendedVec3(pos.x, pos.y, pos.z), 5);
		}

		// Load sunbed
		level.addStaticObject("assets/models/sunbed.obj", PxExtendedVec3(375, getYPosition(375, -220) - 5, -220), 5);

		// Init character
		GLuint animateShader = getComputeShader("assets/shader/animator.comp");
		Character character(textureShader, "assets/models/main_char_animated_larry_4.obj", gPhysicsSDK, gCooking, gScene, mMaterial, pxChar, &playerCamera, gManager, animateShader, viewFrustum);

		// Adjust character to 3d person cam
		for (int i = 0; i < character.nodes.size(); i++) {
			character.nodes[i]->setTransformMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		}
		character.init();

		//Relocate the character & camera
		character.relocate(physx::PxExtendedVec3(terrainPlaneSize / 2, 100.0f, -terrainPlaneSize / 2));


		///* --------------------------------------------- */
		//// Init Particle Sahder
		///* --------------------------------------------- */
		//GLuint renderProgram = getParticleShader("assets/shader/particle.vert", "assets/shader/particle.geom", "assets/shader/particle.frag");
		//GLuint computeShader = getComputeShader("assets/shader/particle.comp");
		//Particle particleShader(computeShader, renderProgram, camera.getProjection());
		//particleShader.init();
		/* GAMEPLAY END */


		/* GAMEPLAY */
		double xpos = 0;
		double ypos = 0;
		double xRotate = 0;
		double yRotate = 0;
		float t = float(glfwGetTime());
		float t2 = t;
		float dt = 0.0f;
		float t_sum = 0.0f;
		int waitingMS = 0;
		PxReal timeStep = 1.0f / 60.0f;
		float timeStepFloat = 1.0f / 60.0f;
		std::shared_ptr<Enemy> selectedEnemy = nullptr;
		float strongCD = 0.0f;
		float areaCD = 0.0f;
		float topRightScreen = window_width - (window_width / 5.0f);
		std::string info = "";
		float infoTime = 0.0f;
		int fps = 60;
		int fpsCnt = 0;
		boolean drawFire = false;
		float animationStepBuffer = 0.0f;
		int animationStep = 0;
		bool is_moving = false;
		/* GAMEPLAY END */

		while (!glfwWindowShouldClose(window)) {
			// Clear backbuffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Poll events -> move to end (GAMEPLAY)
			//glfwPollEvents();

			/* GAMEPLAY */
			gScene->simulate(timeStep);
			gScene->fetchResults(true);

			// Update camera
			playerCamera.updateZoom(_zoom);

			if (_dragging || _draggingCamOnly) {
				glfwGetCursorPos(window, &xpos, &ypos);
				xRotate = (xpos - lastxpos) * 0.3333f;
				yRotate = (ypos - lastypos) * 0.5f;

				lastxpos = xpos;
				lastypos = ypos;

				playerCamera.rotate(-xRotate, -yRotate);
				if (!_draggingCamOnly) {
					character.updateRotation(playerCamera.getYaw());
				}
			}
			/* GAMEPLAY END */

			// Update camera
			//glfwGetCursorPos(window, &mouse_x, &mouse_y);
			//camera.update(int(mouse_x), int(mouse_y), _zoom, _dragging, _strafing);

			// Set per-frame uniforms
			setPerFrameUniformsNormal(textureShader.get(), playerCamera, pointL);

			/* GAMEPLAY */
			// update character and camera position
			is_moving = move_character(window, &character, dt);

			// update all enemy positions and deaths
			for (size_t i = 0; i < level.enemies.size(); i++) {
				level.enemies[i]->chase(character.getPosition(), dt);
			}

			// update view frustum
			viewFrustum->doCheck = _checkFrustum;
			if (_checkFrustum) {
				camModel = playerCamera.getModel();
				viewFrustum->setCamDef(getWorldPosition(camModel), getLookVector(camModel), getUpVector(camModel));
			}

			// _hitDetection from physx callback -> locked on 60 fps
			if (_hitDetection) {
				character.inflictDamage(1);
				_hitDetection = false;
			}

			/* GAMEPLAY END */
			
			setPerFrameUniforms(tessellationShader.get(), playerCamera, pointL);


			// 1. render depth of scene to texture (from light's perspective)
			// --------------------------------------------------------------
			shadowMap.updateLightPos(pointL.position);
			shadowMap.draw();
			level.drawDepth(shadowMapDepthShader.get());
			//for (glm::vec3 pos : points)
			//{
			//	treePlaceholderDepth.resetModelMatrix();
			//	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - terrainPlaneSize / 2, pos.y + 150, pos.z - terrainPlaneSize / 2));
			//	treePlaceholderDepth.transform(transformation);
			//	treePlaceholderDepth.draw();
			//	
			//}
			planeShadow.draw(shadowMapDepthShader.get());
			
			shadowMap.unbindFBO();

			// reset viewport
			glViewport(0, 0, window_width, window_height);
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// debug: render shadowMap on quad 
			//shadowMap.drawDebug(shadowMapDebugShader.get());
			//renderQuad();


			// 2. Render Scene
			// --------------------------------------------------------------
			// Render Skybox
			skybox.draw(playerCamera, brightness);

			// Render terrain
			plane.draw(tessellationShader.get(), playerCamera, shadowMap, brightness);

			/* GAMEPLAY */
			level.draw();
			character.animate(animationStep);
			/* GAMEPLAY END*/

			// Render flares
			flareMangaer.render(playerCamera.getViewProjectionMatrix(), pointL.position, brightness);

			// Render GUI
			guiRenderer.render(guis, brightness);

			// Compute frame time
			dt = t;
			t = float(glfwGetTime());
			dt = t - dt;
			t_sum += dt;

			/* GAMEPLAY */
			animationStepBuffer += dt;
			if (animationStepBuffer >= timeStepFloat) {
				animationStepBuffer = timeStepFloat - animationStepBuffer;
				if (is_moving) {
					animationStep = (animationStep + 1); // % 60;
				}
				else {
					animationStep = 0;
				}

			}
			if (_limitFPS) {
				waitingMS = (1000 / _selectedFPS) - (t - t2) * 1000;
				if (waitingMS > 0) {
					Sleep(waitingMS);
				}
				t2 = float(glfwGetTime());
			}

			// Poll events
			glfwPollEvents();
			/* GAMEPLAY END */
			
			// Swap buffers
			glfwSwapBuffers(window);
		}
	}


	/* --------------------------------------------- */
	// Destroy framework
	/* --------------------------------------------- */
	/* GAMEPLAY */
	gScene->release();
	gPhysicsSDK->release();
	gFoundation->release();
	pvd->release();
	transport->release();
	FreeImage_DeInitialise();
	/* GAMEPLAY END */
	destroyFramework();


	/* --------------------------------------------- */
	// Destroy context and exit
	/* --------------------------------------------- */

	glfwTerminate();

	return EXIT_SUCCESS;
}

float getYPosition(float x, float z) {

	glm::vec2 pixelPos = glm::vec2(floor(x / terrainPlaneSize * imgWidth), floor(-(z) / terrainPlaneSize * imgHeight));
	unsigned char* value = data + 4 * int((pixelPos.y * imgWidth + pixelPos.x));
	float redChannel = float(value[0]) / 255;
	return redChannel * terrainHeight;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


void setPerFrameUniformsNormal(Shader* shader, PlayerCamera& camera, PointLight& pointL)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("pointL.color", pointL.color);
	shader->setUniform("pointL.position", pointL.position);
	shader->setUniform("pointL.attenuation", pointL.attenuation);
	shader->setUniform("brightness", brightness);
	
	shader->unuse();
}

void setPerFrameUniforms(TerrainShader* shader, PlayerCamera& camera, PointLight& pointL)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("lightPosition", pointL.position);
}

/* GAMEPLAY */
bool move_character(GLFWwindow* window, Character* character, float deltaMovement) {
	float forward = 0;
	float leftStrafe = 0;
	bool updateForward = false;
	bool updateStrafe = false;

	int holdingForward = glfwGetKey(window, GLFW_KEY_W);
	int holdingBackward = glfwGetKey(window, GLFW_KEY_S);
	int holdingLeftStrafe = glfwGetKey(window, GLFW_KEY_A);
	int holdingRightStrafe = glfwGetKey(window, GLFW_KEY_D);

	if (holdingForward == GLFW_PRESS) {
		forward += deltaMovement;
		updateForward = true;
	}
	if (holdingBackward == GLFW_PRESS) {
		forward -= deltaMovement;
		updateForward = true;
	}

	if (holdingLeftStrafe == GLFW_PRESS) {
		leftStrafe += deltaMovement;
		updateStrafe = true;
	}
	if (holdingRightStrafe == GLFW_PRESS) {
		leftStrafe -= deltaMovement;
		updateStrafe = true;
	}

	if (updateForward || updateStrafe) {
		if (!updateStrafe) {
			forward *= 1.414f;
		}
		if (!updateForward) {
			leftStrafe *= 1.414f;
		}
		character->move(-forward * 35.0f, -leftStrafe * 35.0f, deltaMovement);
		return true;
	}
	return false;
}
/* GAMEPLAY END */


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
	//	_dragging = true;
	//}
	//else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	//	_dragging = false;
	//}
	//else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
	//	_strafing = true;
	//}
	//else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
	//	_strafing = false;
	//}
	/* GAMEPLAY */
	{
		/*	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
				glfwGetCursorPos(window, &lastxpos, &lastypos);
				_dragging = true;
				_draggingCamOnly = false;
			} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
				_dragging = false;
				_draggingCamOnly = false;
			} else*/ if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
				glfwGetCursorPos(window, &lastxpos, &lastypos);
				_draggingCamOnly = true;
				_mouseSelect = true;
			}
			else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
				_draggingCamOnly = false;
			}
	}
	/* GAMEPLAY END */
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	_zoom -= float(yoffset) * 20.5f; //300.0f
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//// F1 - Wireframe
	//// F2 - Culling
	//// Esc - Exit

	//if (action != GLFW_RELEASE) return;

	//switch (key)
	//{
	//case GLFW_KEY_ESCAPE:
	//	glfwSetWindowShouldClose(window, true);
	//	break;
	//case GLFW_KEY_F1:
	//	_wireframe = !_wireframe;
	//	glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
	//	break;
	//case GLFW_KEY_F2:
	//	_culling = !_culling;
	//	if (_culling) glEnable(GL_CULL_FACE);
	//	else glDisable(GL_CULL_FACE);
	//	break;
	//}
	/* GAMEPLAY */
	
	// F2 - FPS Limiter
	// F3 - Wireframe ON/OFF
	
	// F8 - View Frustum ON/OFF
	// F9 - Backface Culling



	// Esc - Exit

	if (action != GLFW_RELEASE) return;

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	//case GLFW_KEY_F1:
	//	_showHelp = !_showHelp;
	//	break;
	case GLFW_KEY_F2:
		_limitFPS = !_limitFPS;
		break;
	case GLFW_KEY_F3:
		_wireframe = !_wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
		break;
	case GLFW_KEY_F8:
		_checkFrustum = !_checkFrustum;
		break;
	case GLFW_KEY_F9:
		_culling = !_culling;
		if (_culling) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);
		break;
	}
	/* GAMEPLAY END */
}

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	if (id == 131185 || id == 131218) return; // ignore performance warnings from nvidia
	std::string error = FormatDebugOutput(source, type, id, severity, message);
	std::cout << error << std::endl;
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}

	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}