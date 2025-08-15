#pragma once
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>
#include "particle.hpp"
#include "shaders.hpp"
#include "camera.hpp"


constexpr int WinWidth = 800;
constexpr int WinHeight = 600;
constexpr int rows = 10;
constexpr int cols = 10;
constexpr float spacing = 0.1f;

namespace fs = std::filesystem;

class Simulation {
public:
	Simulation();
	bool init();
	void run();

private:
	std::vector<Particle> particles;
	Camera camera;
	Shader particleShader;
	GLuint particleVAO, particleVBO;
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
	void processEvent();
	void render();
	void framebuffer_size_callback(int width, int height);
	void clean();
};







