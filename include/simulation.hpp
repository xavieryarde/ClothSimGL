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
constexpr int cols = 30;
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

private:
	void initUBO();
	void initParticle();
	void initSprings();
	void processEvent();
	void render();
	void framebuffer_size_callback(int width, int height);
	void clean();
};







