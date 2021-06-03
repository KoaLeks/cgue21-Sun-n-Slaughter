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


/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setPerFrameUniformsNormal(Shader* shader, Camera& camera, PointLight& pointL, Texture& hm);
void setPerFrameUniforms(TerrainShader* shader, Camera& camera, PointLight& pointL);
void renderQuad();


/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool _wireframe = false;
static bool _culling = true;
static bool _dragging = false;
static bool _strafing = false;
static float _zoom = 5.0f;
static int sreen_width = 1600;
static int sreen_height= 900;

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


	/* --------------------------------------------- */
	// Init framework
	/* --------------------------------------------- */

	if (!initFramework()) {
		EXIT_WITH_ERROR("Failed to init framework");
	}

	// set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// set GL defaults
	glClearColor(1, 1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

		int terrainPlane = 10000;
		int terrainHeight = 2000;
		float lightDistance = 20000.0f;

		// Create Terrain
		// heightmap muss ein vielfaches von 20 (oder 2^n?) sein, ansonsten wirds nicht korrekt abgebildet
		Terrain plane = Terrain(terrainPlane, 50, terrainHeight, "assets/terrain/heightmap2.png", false);
		Terrain planeShadow = Terrain(terrainPlane, 50, terrainHeight, "assets/terrain/heightmap2.png", true);

		// Create Skybox
		Skybox skybox = Skybox(skyboxShader.get());

		// Initialize camera
		Camera camera(fov, float(window_width) / float(window_height), nearZ, farZ);
		camera.update(window_width, window_height, false, false, false);

		// Initialize lights
		//PointLight pointL(glm::vec3(.5f), glm::vec3(50000, lightDistance, 0), glm::vec3(0.08f, 0.03f, 0.01f));
		PointLight pointL(glm::vec3(.5f), glm::vec3(-15000, 15000, -25000), glm::vec3(0.08f, 0.03f, 0.01f));

		// Shadow Map
		ShadowMap shadowMap = ShadowMap(shadowMapDepthShader.get(), pointL.position, nearZ, farZ / 10, 5000.0f);

		std::shared_ptr<MeshMaterial> material = std::make_shared<MeshMaterial>(textureShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);
		std::shared_ptr<MeshMaterial> depth = std::make_shared<MeshMaterial>(shadowMapDepthShader, glm::vec3(0.5f, 0.7f, 0.3f), 8.0f);
		
		Mesh light = Mesh(glm::translate(glm::mat4(1.0f), pointL.position), Mesh::createSphereMesh(12, 12, lightDistance / 15), material);
		Mesh sun = Mesh(glm::translate(glm::mat4(1.0f), glm::vec3(-15000, 15000, -25000)), Mesh::createSphereMesh(12, 12, 2500), material);
		Mesh sphere3Depth = Mesh(glm::translate(glm::mat4(1.0f), glm::vec3(-700, 2000, -600)), Mesh::createSphereMesh(12, 12, 450), depth);
		Mesh sphere3 = Mesh(glm::translate(glm::mat4(1.0f), glm::vec3(2000, 2000, 1200)), Mesh::createSphereMesh(12, 12, 450), material);
		

		// Tree positions
		Texture heightMap = Texture("assets/terrain/heightmap2.png", true);
		PossionDiskSampling treePositions = PossionDiskSampling(terrainPlane, terrainPlane, 350, 30);
		treePositions.applyMask("assets/terrain/treemask.png");
		std::vector<glm::vec2> points = treePositions.getPoints();
		Mesh test = Mesh(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)), Mesh::createSphereMesh(12, 12, 150), material);
		Mesh testDepth = Mesh(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)), Mesh::createSphereMesh(12, 12, 150), depth);


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


		// Render loop
		float t = float(glfwGetTime());
		float dt = 0.0f;
		float t_sum = 0.0f;
		double mouse_x, mouse_y;

		while (!glfwWindowShouldClose(window)) {
			// Clear backbuffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Poll events
			glfwPollEvents();

			// Update camera
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			camera.update(int(mouse_x), int(mouse_y), _zoom, _dragging, _strafing);

			// Set per-frame uniforms
			setPerFrameUniformsNormal(textureShader.get(), camera, pointL, heightMap);
			
			// Moving light
			//pointL.position.x = cos(t / 2) * terrainPlane;
			//if (sin(t / 2) > 0) {
			//	pointL.position.y = sin(t / 2) * terrainPlane / 2;
			//}
			//else
			//{
			//	pointL.position.y = -sin(t / 2) * terrainPlane / 2;
			//}
			//light.resetModelMatrix();
			//light.transform(glm::translate(glm::mat4(1), glm::vec3(pointL.position.x, pointL.position.y, pointL.position.z)));
			
			setPerFrameUniforms(tessellationShader.get(), camera, pointL);


			// 1. render depth of scene to texture (from light's perspective)
			// --------------------------------------------------------------
			shadowMap.updateLightPos(pointL.position);
			shadowMap.draw();
			sphere3Depth.draw();
			planeShadow.draw(shadowMapDepthShader.get());
			for (glm::vec2 pos : points)
			{
				testDepth.resetModelMatrix();
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - terrainPlane / 2, 150, pos.y - terrainPlane / 2));
				testDepth.transform(transformation);
				testDepth.draw();
			}
			
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
			skybox.draw(camera);

			// Render terrain
			plane.draw(tessellationShader.get(), camera, shadowMap);
			
			// light sphere
			//light.draw();
			//sun.draw();
			//sphere3.draw();
			
			// Render trees
			for (glm::vec2 pos :points )
			{		
				test.resetModelMatrix();
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - terrainPlane / 2, 150, pos.y - terrainPlane / 2));
				test.transform(transformation);
				test.draw();
				
			}

			// Render flares
			flareMangaer.render(camera.getViewProjectionMatrix(), pointL.position);

			// Render GUI
			guiRenderer.render(guis);

			// Compute frame time
			dt = t;
			t = float(glfwGetTime());
			dt = t - dt;
			t_sum += dt;
			
			// Swap buffers
			glfwSwapBuffers(window);
		}
	}


	/* --------------------------------------------- */
	// Destroy framework
	/* --------------------------------------------- */

	destroyFramework();


	/* --------------------------------------------- */
	// Destroy context and exit
	/* --------------------------------------------- */

	glfwTerminate();

	return EXIT_SUCCESS;
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


void setPerFrameUniformsNormal(Shader* shader, Camera& camera, PointLight& pointL, Texture& hm)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("scaleXZ", 10000.f);
	shader->setUniform("scaleY", 2000.f);

	hm.bind(7);
	shader->setUniform("heightMap", 7);
	//shader->setUniform("dirL.color", glm::vec3(1));
	//shader->setUniform("dirL.direction", glm::vec3(1));
	shader->setUniform("pointL.color", pointL.color);
	shader->setUniform("pointL.position", pointL.position);
	shader->setUniform("pointL.attenuation", pointL.attenuation);
	
	shader->unuse();
}

void setPerFrameUniforms(TerrainShader* shader, Camera& camera, PointLight& pointL)
{
	shader->use();
	shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
	shader->setUniform("camera_world", camera.getPosition());
	shader->setUniform("lightPosition", pointL.position);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		_dragging = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		_dragging = false;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		_strafing = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		_strafing = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	_zoom -= float(yoffset) * 300.f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// F1 - Wireframe
	// F2 - Culling
	// Esc - Exit

	if (action != GLFW_RELEASE) return;

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_F1:
		_wireframe = !_wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
		break;
	case GLFW_KEY_F2:
		_culling = !_culling;
		if (_culling) glEnable(GL_CULL_FACE);
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