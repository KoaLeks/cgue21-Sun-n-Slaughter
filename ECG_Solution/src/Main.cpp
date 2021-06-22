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
#include "TextRenderer.h"
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
bool move_character(GLFWwindow* window, Character* character, PlayerCamera* playerCamera, float deltaMovement);
/* GAMEPLAY END */
void setPerFrameUniformsNormal(Shader* shader, PlayerCamera& camera, PointLight& pointL, ShadowMap& shadowMap);
void setPerFrameUniforms(TerrainShader* shader, PlayerCamera& camera, PointLight& pointL, ShadowMap& shadowMap);
int main(int argc, char** argv);
float getYPosition(float x, float z);
void renderQuad();
void loadHighscores();
void saveHighscore();
void showHighscores(TextRenderer* hud, glm::vec3 color = glm::vec3(1.0f));
void showHelp(TextRenderer* hud, glm::vec3 color = glm::vec3(1.0f));


/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static int window_width = 1600;
static int window_height = 900;

static char* heightMapPath = "assets/terrain/hm3.png";
static char* treeMaskPath = "assets/terrain/mask1.png";

/* GAMEPLAY */
static bool checkWireframe = false;
static bool checkBackCulling = true;
static bool checkVFC = true;
static bool help = false;
bool checkShadows = true;
static bool checkFPSLimit = false;

static float _fov = 60.0f;
double lastxpos = 0;
double lastypos = 0;
int selectedFPS = 60;


bool hitDetection = false;
int enemyDetection = -1;

bool dashInProgress = false;
float dashDuration = 0.5f;
float dashCoolDown = 0.0f; //Max 10.0f


bool disableTextures = false;
float brightness = 1.0;
int imgWidth, imgHeight, nrChannels;
unsigned char* data;
float playerSpeed = 30.f;
float enemySpeed = 20.f;

int terrainPlaneSize = 1024;
int terrainHeight = 250;

std::string playerName;
long long highscore = 0;
std::string highscoresN[5];
long long highscores[5];
bool isSaved = false;
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

	window_width = reader.GetInteger("window", "width", 1600);
	window_height = reader.GetInteger("window", "height", 900);
	int refresh_rate = reader.GetInteger("window", "refresh_rate", 60);
	bool fullscreen = reader.GetBoolean("window", "fullscreen", false);
	std::string window_title = reader.Get("window", "title", "Sun'n'Slaughter");
	_fov = float(reader.GetReal("camera", "fov", 60.0f));
	float nearZ = float(reader.GetReal("camera", "near", 0.1f));
	float farZ = float(reader.GetReal("camera", "far", 100000.0f));
	brightness = float(reader.GetReal("window", "brightness", 1.0));

	/* GAMEPLAY */
	selectedFPS = reader.GetInteger("window", "fps", 60);

	playerName = reader.Get("player", "name", "Unknown");
	/* GAMEPLAY END */

	//Load highscores
	loadHighscores();

	//TEST
	//highscore = 99;
	//saveHighscore();


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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	// Init HUD / Text / SPLASHSCREEN
	/* --------------------------------------------- */

	TextRenderer* hud = new TextRenderer(window_width, window_height);
	hud->Load("assets/fonts/beachday.ttf", 32);
	//splashscreen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hud->RenderText(window_title,
		window_width / 2 - 225, window_height / 2 - 100.0f, 2.0f, glm::vec3(1, 0, 0));
	showHighscores(hud, glm::vec3(0.0));
	showHelp(hud, glm::vec3(0.0));
	glfwSwapBuffers(window);

	/* --------------------------------------------- */
	// Init Physx
	/* --------------------------------------------- */

	PxDefaultErrorCallback gDefaultErrorCallback;
	PxDefaultAllocator gDefaultAllocatorCallback;
	PxFoundation* gFoundation = nullptr;
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

	PxPvd* pvd = PxCreatePvd(*gFoundation);
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

	SimulationCallback* simulationCallback = new SimulationCallback(&hitDetection, &enemyDetection, &dashInProgress);
	PxScene* gScene = nullptr;
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
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
	//cDesc.reportCallback = simulationCallback;
	PxController* pxChar = gManager->createController(cDesc);
	pxChar->getActor()->setName("larry");

	/* GAMEPLAY END */


	/* --------------------------------------------- */
	// Initialize scene and render loop
	/* --------------------------------------------- */
	{

		// Load shader(s)
		std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("texture.vert", "texture.frag");
		std::shared_ptr<Shader> debugShader = std::make_shared<Shader>("debug.vert", "debug.frag");
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
		//PointLight pointL(glm::vec3(.5f), glm::vec3(-1500, 1700, -2500), glm::vec3(0.08f, 0.03f, 0.01f));
		PointLight pointL(glm::vec3(.5f), glm::vec3(-900, 1020, -1500), glm::vec3(0.08f, 0.03f, 0.01f));

		// Shadow Map
		ShadowMap shadowMap = ShadowMap(shadowMapDepthShader.get(), pointL.position, nearZ, farZ, 400.0f, glm::vec3(terrainPlaneSize / 2, 0, -terrainPlaneSize / 2));

		std::shared_ptr<MeshMaterial> debug = std::make_shared<MeshMaterial>(debugShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);
		std::shared_ptr<MeshMaterial> material = std::make_shared<MeshMaterial>(textureShader, glm::vec3(0.3f, 0.8f, 0.0f), 8.0f);
		std::shared_ptr<MeshMaterial> depth = std::make_shared<MeshMaterial>(shadowMapDepthShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);

		Mesh frust = Mesh(glm::translate(glm::mat4(1), glm::vec3(0)), Mesh::createCubeMesh(1, 1, 1), debug);

		// Tree positions
		PossionDiskSampling treePositions = PossionDiskSampling(terrainPlaneSize, treeMaskPath, heightMapPath, terrainHeight, 80, 10);
		std::vector<glm::vec3> points = treePositions.getPoints();

		// GUI
		//std::vector<GuiTexture> guis;
		//Texture heart = Texture("assets/heart.png", true);
		//float scaleFactor = 50.0f;
		//glm::vec2 scale = glm::vec2(scaleFactor * heart.getAspectRatio() / window_width, scaleFactor / window_height);
		//GuiTexture gui1 = GuiTexture(heart.getTextureId(), glm::vec2(-0.95f, 0.9f), scale);
		//GuiTexture gui2 = GuiTexture(heart.getTextureId(), glm::vec2(-0.88f, 0.9f), scale);
		//GuiTexture gui3 = GuiTexture(heart.getTextureId(), glm::vec2(-0.81f, 0.9f), scale);
		//GuiTexture gui4 = GuiTexture(heart.getTextureId(), glm::vec2(-0.74f, 0.9f), scale);
		//guis.push_back(gui1);
		//guis.push_back(gui2);
		//guis.push_back(gui3);
		//guis.push_back(gui4);
		//GuiRenderer guiRenderer = GuiRenderer(guiShader.get());

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
		PlayerCamera playerCamera(_fov, float(window_width) / float(window_height), nearZ, farZ);
		std::shared_ptr<FrustumG> viewFrustum = std::make_shared<FrustumG>();
		viewFrustum->setDebugMesh(frust);
		viewFrustum->setCamInternals(_fov, float(window_width) / float(window_height), nearZ, farZ);
		glm::mat4 camModel = playerCamera.getModel();
		viewFrustum->setCamDef(getWorldPosition(camModel), getLookVector(camModel), getUpVector(camModel));

		//std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("texture.vert", "texture.frag");
		Scene level(textureShader, "assets/models/cook_map_detailed.obj", gPhysicsSDK, gCooking, gScene, mMaterial, gManager, viewFrustum);

		// Load heightmap
		data = stbi_load(heightMapPath, &imgWidth, &imgHeight, &nrChannels, 4);

		// Frustum TEST
		//level.addStaticObject("assets/models/trees/palmTree.obj", PxExtendedVec3(-4054, 1430, -559), 20); // top left
		//level.addStaticObject("assets/models/trees/palmTree.obj", PxExtendedVec3(3429, 1468, 3323), 20); // bottom right

		// Load trees
		for (glm::vec3 pos : points)
		{
			pos.z -= terrainPlaneSize;
			level.addStaticObject("assets/models/trees/palmTree.obj", PxExtendedVec3(pos.x, pos.y, pos.z), 5);
		}

		// Load sunbed
		level.addStaticObject("assets/models/sunbed.obj", PxExtendedVec3(375, getYPosition(375, -220) - 5, -220), 3);

		// TEST ENEMY
		//level.addEnemy(physx::PxExtendedVec3(500, getYPosition(500, -500), -500), 10, simulationCallback);
		//level.addEnemy(physx::PxExtendedVec3(550, getYPosition(550, -500), -500), 10, simulationCallback);
		//level.addEnemy(physx::PxExtendedVec3(450, getYPosition(450, -500), -500), 10, simulationCallback);
		//level.addEnemy(physx::PxExtendedVec3(300, getYPosition(300, -500), -500), 10, simulationCallback);
		
		//Add enemys
		// bot left, top left, top right, bot right
		level.addEnemy(physx::PxExtendedVec3(50, getYPosition(50, -50)    + 5, -50)   , 10, simulationCallback);
		level.addEnemy(physx::PxExtendedVec3(950, getYPosition(950, -50)  + 5, -50) , 10, simulationCallback);
		level.addEnemy(physx::PxExtendedVec3(950, getYPosition(950, -950) + 5, -950), 10, simulationCallback);
		level.addEnemy(physx::PxExtendedVec3(50, getYPosition(50, -950)   + 5, -950) , 10, simulationCallback);

		// Init character
		GLuint animateShader = getComputeShader("assets/shader/animator.comp");
		
		Character character(textureShader, "assets/models/main_char_animated_larry_4.obj", gPhysicsSDK, gCooking, gScene, mMaterial, pxChar, &playerCamera, gManager, animateShader, viewFrustum);

		// Adjust character to 3d person cam
		for (int i = 0; i < character.nodes.size(); i++) {
			character.nodes[i]->setTransformMatrix(glm::rotate(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1)), glm::vec3(0.0f, -2.0f, 0.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
		}
		character.init();

		//Relocate the character & camera
		character.relocate(physx::PxExtendedVec3(370, 104, -223));


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
		float fps_start = 0.0f;
		float fps_current = 0.0f;
		float fps_delta = 0.0f;
		float fps_update = 5.0f; // 5 updates per second
		int waitingMS = 0;
		PxReal timeStep = 1.0f / 60.0f;
		float timeStepFloat = 1.0f / 60.0f;
		std::shared_ptr<Enemy> selectedEnemy = nullptr;
		std::string info = "";
		float infoTime = 0.0f;
		int fps = 0;
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
			playerCamera.updateZoom(_fov);

			if (/*_dragging || _draggingCamOnly*/true) {
				glfwGetCursorPos(window, &xpos, &ypos);
				xRotate = (xpos - lastxpos) * 0.3333f;
				yRotate = (ypos - lastypos) * 0.5f;

				lastxpos = xpos;
				lastypos = ypos;

				playerCamera.rotate(-xRotate, -yRotate);
				if (/*!_draggingCamOnly*/true) {
					character.updateRotation(playerCamera.getYaw());
				}
				//this is the main thing that keeps it from leaving the screen
				if (xpos < 100 || xpos > window_width - 100) {
					lastxpos = window_width / 2;
					lastypos = window_height / 2;
					glfwSetCursorPos(window, lastxpos, lastypos);
				}
				else if (ypos < 100 || ypos > window_height - 100) {
					lastxpos = window_width / 2;
					lastypos = window_height / 2;
					glfwSetCursorPos(window, lastxpos, lastypos);
				}
			}
			/* GAMEPLAY END */

			// Update camera
			//glfwGetCursorPos(window, &mouse_x, &mouse_y);
			//camera.update(int(mouse_x), int(mouse_y), _zoom, _dragging, _strafing);

			// Set per-frame uniforms
			setPerFrameUniformsNormal(textureShader.get(), playerCamera, pointL, shadowMap);
			setPerFrameUniformsNormal(debugShader.get(), playerCamera, pointL, shadowMap);

			/* GAMEPLAY */
			// update character and camera position
			is_moving = move_character(window, &character, &playerCamera, dt);

			// update all enemy positions and deaths
			for (size_t i = 0; i < level.enemies.size(); i++) {
				level.enemies[i]->chase(character.getPosition(), enemySpeed, dt);
			}

			//dash attack
			if (dashInProgress) {
				dashCoolDown = 10.0f;
				dashDuration -= dt;
				if (dashDuration < 0.0f) {
					dashInProgress = false;
					dashDuration = 1.0f;
				}
			}
			else {
				if (dashCoolDown > 0.0f) {
					dashCoolDown -= dt;
				}
			}

			// hitDetection from physx callback -> locked on 60 fps
			if (hitDetection) {
				character.inflictDamage(1);
				hitDetection = false;
			}

			/* GAMEPLAY END */

			setPerFrameUniforms(tessellationShader.get(), playerCamera, pointL, shadowMap);


			// 1. render depth of scene to texture (from light's perspective)
			// --------------------------------------------------------------
			if (checkShadows) {
				shadowMap.updateLightPos(pointL.position * glm::vec3(0.5));

				//glCullFace(GL_FRONT);
				shadowMap.draw();
				character.drawDepth(shadowMapDepthShader.get());
				level.drawDepth(shadowMapDepthShader.get());
				//glCullFace(GL_BACK);
				planeShadow.draw(shadowMapDepthShader.get());

				shadowMap.unbindFBO();

				// reset viewport
				glViewport(0, 0, window_width, window_height);
				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				// debug: render shadowMap on screen 
				//shadowMap.drawDebug(shadowMapDebugShader.get());
				//renderQuad();
			}

			// update view frustum
			viewFrustum->doCheck = checkVFC;
			if (checkVFC) {
				camModel = (playerCamera.getModel());
				viewFrustum->updateFOV(_fov);
				viewFrustum->setCamDef(playerCamera.getActualPosition(), getLookVector(camModel), getUpVector(camModel));
			}

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

			// Compute frame time
			dt = t;
			t = float(glfwGetTime());
			dt = t - dt;
			t_sum += dt;


			fpsCnt++;
			fps_delta += dt;
			if (fps_delta > 1.0 / fps_update) {
				fps = int(fpsCnt / fps_delta);
				fpsCnt = 0;
				fps_delta -= 1.0 / fps_update;
			}

			//win or rather lose condition
			if (character.getHP() <= 0) {
				if (!isSaved) { //TODO isSaved for general blocking of controlls after death?
					saveHighscore(); // TODO: Add text if you got an new highscore
					isSaved = true;
				}
				hud->RenderText("You got slaughtered!", window_width / 2 - 300, window_height / 2 - 100.0f, 2.0f, glm::vec3(1, 0, 0));
				hud->RenderText("Highscore: " + std::to_string(highscore), 
					window_width - (170 + 16 * std::to_string(highscore).length()), 15.0f, 1.0f, glm::vec3(1, 0, 0));
				showHighscores(hud, glm::vec3(1.0f, 0.0f, 0.0f));
				brightness -= dt / 4;

				if (brightness <= -1.0f) {
					glfwSetWindowShouldClose(window, true);
				}
			}
			else {
				// draw HUD
				hud->RenderText("HP: " + std::to_string(character.getHP()), 15.0f, 15.0f, 1.0f, (character.getHP() < 25) ? glm::vec3(1, 0, 0) : glm::vec3(1));
				hud->RenderText("Dash: " + std::to_string(int(10.0 - dashCoolDown)) + "/10", 15.0f, 55.0f, 1.0f, (dashCoolDown > 0.0) ? glm::vec3(1, 0, 0) : glm::vec3(1));
				hud->RenderText("Highscore: " + std::to_string(highscore), window_width - (170 + 16 * std::to_string(highscore).length()), 15.0f, 1.0f);

				if (help) {
					showHighscores(hud);
					showHelp(hud);
				}

				hud->RenderText("FPS: " + std::to_string(fps), 15.0f, window_height - 35.0f, 1.0f);
				hud->RenderText("Objects: " + std::to_string(level.getDrawnObjects()), 15.0f, window_height - 75.0f, 1.0f);
			}

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
			if (checkFPSLimit) {
				waitingMS = (1000 / selectedFPS) - (t - t2) * 1000;
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

void loadHighscores() {
	// Create a text string, which is used to output the text file
	std::string line;
	int place = 0; // first place (1) = 0

	// Read from the text file
	std::ifstream file("assets/highscores.txt");

	// Use a while loop together with the getline() function to read the file line by line
	while (std::getline(file, line)) {
		// Output the text from the file
		highscoresN[place] = line;
		if (std::getline(file, line)) {
			highscores[place] = std::atoi(line.c_str());
		}
		//std::cout << highscoresN[place] << ": " << highscores[place] << std::endl;
		place++;
	}

	// Close the file
	file.close();
}

void saveHighscore() {
	bool saved = false;
	std::ofstream file("assets/highscores.txt");

	for (int i = 0; i < 5; i++) {
		if (highscores[i] < highscore && !saved) {
			for (int x = 3; x >= i; x--) {
				highscores[x + 1] = highscores[x];
				highscoresN[x + 1] = highscoresN[x];
			}

			highscores[i] = highscore;
			highscoresN[i] = playerName;

			saved = true;
		}
		file << highscoresN[i] << std::endl;
		file << highscores[i] << std::endl;
	}

	// Close the file
	file.close();
}

void showHighscores(TextRenderer* hud, glm::vec3 color)
{
	std::string line = "";

	hud->RenderText("Highscores:", 15.0f, window_height / 2 - 200.0f, 1.0f, color);
	for (int i = 0; i < 5; i++) {
		line = std::to_string(i + 1) + ". " + highscoresN[i] + "..." + std::to_string(highscores[i]);
		hud->RenderText(line, 15.0f, (window_height / 2 - 150.0f) + 40 * i, 0.8f, color);
	}
}

void showHelp(TextRenderer* hud, glm::vec3 color) {
	float width = window_width / 2 + 400.0f;
	hud->RenderText("Instructions:", width, 150.0f, 1.0f, color);
	hud->RenderText("F1: Help", width, 200.0f, 0.8f, color);
	hud->RenderText("F2: FPS Limiter " + std::string((checkFPSLimit) ? "(ON)/OFF" : "ON/(OFF)"), width, 240.0f, 0.8f, color);
	hud->RenderText("F3: Wireframe " + std::string((checkWireframe) ? "(ON)/OFF" : "ON/(OFF)"), width, 280.0f, 0.8f, color);
	hud->RenderText("F4: Shadows " + std::string((checkShadows) ? "(ON)/OFF" : "ON/(OFF)"), width, 320.0f, 0.8f, color);
	hud->RenderText("F5: Textures " + std::string((!disableTextures) ? "(ON)/OFF" : "ON/(OFF)"), width, 360.0f, 0.8f, color);
	hud->RenderText("F8: View Frustum Culling " + std::string((checkVFC) ? "(ON)/OFF" : "ON/(OFF)"), width, 400.0f, 0.8f, color);
	hud->RenderText("F9: Backface Culling " + std::string((checkBackCulling) ? "(ON)/OFF" : "ON/(OFF)"), width, 440.0f, 0.8f, color);
	hud->RenderText("Esc: Exit Game", width, 480.0f, 0.8f, color);
	hud->RenderText("---------------------------", width, 520.0f, 0.8f, color);
	hud->RenderText("WASD: Move Player", width, 560.0f, 0.8f, color);
	hud->RenderText("LMB: Standard Attack", width, 600.0f, 0.8f, color);
	hud->RenderText("RMB: Dash Attack", width, 640.0f, 0.8f, color);

	hud->RenderText("Made by Aleksander Marinkovic & Sebastian Karall",
		window_width / 2 - 350, window_height - 30.0f, 1.0f, color);
}

void setPerFrameUniformsNormal(Shader* shader, PlayerCamera& camera, PointLight& pointL, ShadowMap& shadowMap)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("pointL.color", pointL.color);
	shader->setUniform("pointL.position", pointL.position);
	shader->setUniform("pointL.attenuation", pointL.attenuation);
	shader->setUniform("showShadows", checkShadows);
	shader->setUniform("disableTextures", disableTextures);
	shader->setUniform("brightness", brightness);
	shader->setUniform("lightPosition", pointL.position);

	// shadowMap
	shader->setUniform("lightSpaceMatrix", shadowMap.getLightSpaceMatrix());
	shader->setUniform("lightPos", shadowMap.getLightPos());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, shadowMap.getShadowMapID());
	shader->setUniform("shadowMap", 6);
	shader->unuse();
}

void setPerFrameUniforms(TerrainShader* shader, PlayerCamera& camera, PointLight& pointL, ShadowMap& shadowMap)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("showShadows", checkShadows);
	shader->setUniform("disableTextures", disableTextures);
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("lightPosition", pointL.position);

	// shadowMap
	shader->setUniform("lightSpaceMatrix", shadowMap.getLightSpaceMatrix());
	shader->setUniform("lightPos", shadowMap.getLightPos());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, shadowMap.getShadowMapID());
	shader->setUniform("shadowMap", 6);
	shader->unuse();
}

bool move_character(GLFWwindow* window, Character* character, PlayerCamera* playerCamera, float deltaMovement) {
	//float forward = 0;
	//float leftStrafe = 0;
	bool updateForward = false;
	bool updateStrafe = false;

	int holdingForward = glfwGetKey(window, GLFW_KEY_W);
	int holdingBackward = glfwGetKey(window, GLFW_KEY_S);
	int holdingLeftStrafe = glfwGetKey(window, GLFW_KEY_A);
	int holdingRightStrafe = glfwGetKey(window, GLFW_KEY_D);

	float zDir, xDir;
	glm::vec3 up = glm::vec3(0, 1, 0);
	xDir = glm::sin(glm::radians(playerCamera->getYaw()));
	zDir = glm::cos(glm::radians(playerCamera->getYaw()));
	glm::vec3 dirF = glm::normalize(-glm::vec3(xDir, 0, zDir));
	glm::vec3 directionForward(0);
	glm::vec3 directionRight(0);


	if (holdingForward == GLFW_PRESS || dashInProgress) {
		directionForward = dirF;
		updateForward = true;
	}
	else if (holdingForward == GLFW_PRESS && holdingLeftStrafe == GLFW_PRESS && !dashInProgress) {
		directionForward = dirF;
		directionRight = glm::normalize(glm::cross(directionForward, -up));
		updateForward = true;
		updateStrafe = true;
	}
	else if (holdingForward == GLFW_PRESS && holdingRightStrafe == GLFW_PRESS && !dashInProgress) {
		directionForward = dirF;
		directionRight = glm::normalize(glm::cross(directionForward, up));
		updateForward = true;
		updateStrafe = true;
	}

	if (holdingBackward == GLFW_PRESS && !dashInProgress) {
		directionForward = -dirF;
		updateForward = true;
	}
	else if (holdingBackward == GLFW_PRESS && holdingLeftStrafe == GLFW_PRESS && !dashInProgress) {
		directionForward = -dirF;
		directionRight = glm::normalize(glm::cross(-directionForward, -up));
		updateForward = true;
		updateStrafe = true;
	}
	else if (holdingBackward == GLFW_PRESS && holdingRightStrafe == GLFW_PRESS && !dashInProgress) {
		directionForward = -dirF;
		directionRight = glm::normalize(glm::cross(-directionForward, up));
		updateForward = true;
		updateStrafe = true;
	}

	if (holdingLeftStrafe == GLFW_PRESS && !dashInProgress) {
		directionRight = glm::normalize(glm::cross(dirF, -up));
		updateStrafe = true;
	}
	else if (holdingRightStrafe == GLFW_PRESS && !dashInProgress) {
		directionRight = glm::normalize(glm::cross(dirF, up));
		updateStrafe = true;
	}

	//if (holdingForward == GLFW_PRESS) {
	//	forward += deltaMovement;
	//	updateForward = true;
	//}
	//if (holdingBackward == GLFW_PRESS) {
	//	forward -= deltaMovement;
	//	updateForward = true;
	//}
	//
	//if (holdingLeftStrafe == GLFW_PRESS) {
	//	leftStrafe += deltaMovement;
	//	updateStrafe = true;
	//}
	//if (holdingRightStrafe == GLFW_PRESS) {
	//	leftStrafe -= deltaMovement;
	//	updateStrafe = true;
	//}
	//
	if (updateForward || updateStrafe) {
		if (!updateStrafe) {
			directionForward *= 1.414f;
		}
		if (!updateForward) {
			directionRight *= 1.414f;
		}

		//character->move(-forward * playerSpeed, -leftStrafe * playerSpeed, deltaMovement);
		character->move2((directionForward + directionRight), (dashInProgress) ? playerSpeed * 3 : playerSpeed, deltaMovement);

		return true;
	}
	return false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
	//	glfwGetCursorPos(window, &lastxpos, &lastypos);
	//	_draggingCamOnly = true;
	//	_mouseSelect = true;
	//}
	//else 
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		//_draggingCamOnly = false;
		//std::cout << "PRESS" << std::endl;
		dashInProgress = (dashCoolDown > 0.0) ? false : true;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	_fov -= float(yoffset) * 2.0f;
	if (_fov < 30) {
		_fov = 30;
	}
	else if (_fov > 90) {
		_fov = 90;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_RELEASE) return;

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_F1:
		help = !help;
		break;
	case GLFW_KEY_F2:
		checkFPSLimit = !checkFPSLimit;
		break;
	case GLFW_KEY_F3:
		checkWireframe = !checkWireframe;
		glPolygonMode(GL_FRONT_AND_BACK, checkWireframe ? GL_LINE : GL_FILL);
		break;
	case GLFW_KEY_F4:
		checkShadows = !checkShadows;
		break;
	case GLFW_KEY_F5:
		disableTextures = !disableTextures;
		break;
	case GLFW_KEY_F8:
		checkVFC = !checkVFC;
		break;
	case GLFW_KEY_F9:
		checkBackCulling = !checkBackCulling;
		if (checkBackCulling) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);
		break;
	}
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