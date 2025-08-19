#pragma once
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <vector>
#include <cmath>
#include <array>
#include "particle.hpp"
#include "springs.hpp"
#include "shaders.hpp"
#include "camera.hpp"
#include "model.hpp"


constexpr int WinWidth = 800;
constexpr int WinHeight = 600;
constexpr int rows = 30;
constexpr int cols = 40;
constexpr float spacing = 0.18f;

namespace fs = std::filesystem;

enum class SIMMODE {
	TEAR,
	COLLISION,
	FLAG,
	LAST
};

enum class PINNINGMODE {
	TOP_ROW,
	ALL,
	CORNERS,
	FLAG,
	NONE,
	LAST
};

enum class COLLISIONSHAPE {
	CUBE,
	SPHERE,
	LAST
};

struct PoleVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct CollisionObject {
	glm::vec3 position;
	glm::vec3 size; // For cube: width, height, depth. For sphere: radius in x component
	COLLISIONSHAPE shape;
};

class Simulation {
public:
	Simulation();
	bool init();
	void run();

private:
	SIMMODE currentMode;
	PINNINGMODE currentPinning;
	COLLISIONSHAPE currentCollisionShape;
	CollisionObject collisionObject;
	std::vector<Particle> particles;
	std::vector<Spring> springs;
	std::vector<PoleVertex> cylinder;
	std::vector<PoleVertex> cube;
	std::vector<PoleVertex> sphere;
	std::vector<unsigned int> clothIndices;
	std::vector<glm::vec2> clothTexCoords;
	std::vector<unsigned int> flagIndices;
	std::vector<glm::vec2> flagTexCoords;
	Camera camera;
	Shader particleShader;
	Shader clothShader;
	Shader flagShader;
	Shader poleShader;
	Shader skyboxShader;
	GLuint particleVAO, particleVBO;
	GLuint springVAO, springVBO;
	GLuint poleVAO, poleVBO;
	GLuint cubeVAO, cubeVBO;
	GLuint skyboxVAO, skyboxVBO;
	GLuint sphereVAO, sphereVBO;
	GLuint uboMatrices;
	GLuint clothVAO, clothVBO, clothTexVBO, clothNormVBO, clothEBO;
	GLuint flagVAO, flagVBO, flagTexVBO, flagNormVBO, flagEBO;
	unsigned int clothTexture;
	unsigned int flagTexture;
	unsigned int tearCubeMapTexture;
	unsigned int collisionCubeMapTexture;
	unsigned int flagCubeMapTexture;
	bool fullscreen;
	bool running;
	int w, h;
	SDL_Window* window;
	SDL_GLContext context;
	const char* basePath;
	float deltaTime;
	Uint64 lastFrameTime;
	glm::vec2 mousePos;
	bool leftMouseDown;
	float tearRadius;
	std::vector<bool> springActive;
	glm::mat4 projectionMatrix;
	bool isCameraActive;
	std::array<std::string, 6> tearFaces;
	std::array<std::string, 6> collisionFaces;
	std::array<std::string, 6> flagFaces;

private:
	void initUBO();
	void initParticle();
	void initSprings();
	void initClothMesh();
	void initFlagMesh();
	void initSkybox();
	void initCollisionObjects();
	void processEvent();
	void applyPinning();
	void handleMouseActivity();
	void handleMouseTearing();
	void tearSpringsAroundPoint(glm::vec3 worldPos, float radius);
	void handleCollisions();
	bool checkSphereCollision(const glm::vec3& particlePos, float& penetrationDepth, glm::vec3& normal);
	bool checkCubeCollision(const glm::vec3& particlePos, float& penetrationDepth, glm::vec3& normal);
	void resolveCollision(Particle& particle, const glm::vec3& normal, float penetrationDepth);
	glm::vec3 screenToWorld(glm::vec2 screenPos, float depth = 0.0f);
	glm::vec2 worldToScreen(const glm::vec3& worldPos);
	Particle* findClosestParticleToRay(glm::vec3 rayOrigin, glm::vec3 rayDir);
	void render();
	void framebuffer_size_callback(int width, int height);
	unsigned int loadTexture(char const* path);
	unsigned int loadCubemap(std::array<std::string, 6> faces);
	void reset();
	void clean();
	std::vector<glm::vec3> computeNormals(const std::vector<unsigned int>& indices);
	std::vector<PoleVertex> generateCylinder(float radius, float height, int slices);
	std::vector<PoleVertex> generateCube(float size);
	std::vector<PoleVertex> generateSphere(float radius, int rings, int sectors);

};