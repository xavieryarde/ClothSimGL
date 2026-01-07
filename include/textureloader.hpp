#pragma once
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <array>

namespace TextureLoader
{
    unsigned int loadTexture(const char* path);
    unsigned int loadCubemap(const std::array<std::string, 6>& faces);
}