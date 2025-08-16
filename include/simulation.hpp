#pragma once
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>
#include "particle.hpp"
#include "springs.hpp"
#include "shaders.hpp"
#include "camera.hpp"


constexpr int WinWidth = 800;
constexpr int WinHeight = 600;
constexpr int rows = 30;
constexpr int cols = 40;
constexpr float spacing = 0.18f;

namespace fs = std::filesystem;

class Simulation {
public:
	Simulation();
	bool init();
	void run();

private:
	std::vector<Particle> particles;
	std::vector<Spring> springs;
	Camera camera;
	Shader particleShader;
	GLuint particleVAO, particleVBO;
	GLuint springVAO, springVBO;
	GLuint uboMatrices;
	bool fullscreen;
	bool running;
	int w, h;
	SDL_Window* window;
	SDL_GLContext context;
	const char* basePath;
	float deltaTime;
	float lastFrame;
	glm::vec2 mousePos;
	bool leftMouseDown;
	float tearRadius;
	std::vector<bool> springActive;

private:
	void initUBO();
	void initParticle();
	void initSprings();
	void processEvent();
	void handleMouseTearing();
	void tearSpringsAroundPoint(glm::vec3 worldPos, float radius);
	glm::vec3 screenToWorld(glm::vec2 screenPos, float depth = 0.0f);
	glm::vec2 worldToScreen(const glm::vec3& worldPos);
	Particle* findClosestParticleToRay(glm::vec3 rayOrigin, glm::vec3 rayDir);
	void render();
	void framebuffer_size_callback(int width, int height);
	void reset();
	void clean();
};







