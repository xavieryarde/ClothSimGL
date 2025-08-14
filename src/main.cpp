#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

constexpr int WinWidth = 800;
constexpr int WinHeight = 600;

SDL_Window* window = nullptr;
SDL_GLContext context;

bool running = false;
bool fullscreen = false;
int w, h;

bool init();
void processEvent();
void render();
void clean();
void framebuffer_size_callback(int width, int height);


int main(int argc, char* argv[]) {
    if (init()) {
        running = true;
    }
    else {
        SDL_Log("Error Initializing SDL\n");
        return -1;
    }

    while (running) {

        processEvent();

        render();

    }

    clean();

	return 0;
}

bool init() {
    SDL_SetAppMetadata("ClothSimGL", "1.0.0", "com.example.clothsimgl");

    SDL_WindowFlags WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Initialization failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    #ifdef __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    #endif

    window = SDL_CreateWindow("ClothSimGL", WinWidth, WinHeight, WindowFlags);

    if (!window) {
        SDL_Log("SDL Window: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetWindowRelativeMouseMode(window, true)) {
        SDL_Log("SDL Mouse Mode: %s", SDL_GetError());
    }

    context = SDL_GL_CreateContext(window);

    if (!context) {
        SDL_Log("SDL Context: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        SDL_Log("Failed to initialize GLAD\n");
        return false;
    }

    return true;
}

void processEvent() {
    SDL_Event event;
    SDL_zero(event);

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_KEY_DOWN)
        {

            switch (event.key.key)
            {
            case SDLK_ESCAPE:
                running = false;
                break;
            case SDLK_F:
                fullscreen = !fullscreen;
                if (fullscreen)
                {
                    SDL_SetWindowFullscreen(window, true);
                }
                else
                {
                    SDL_SetWindowFullscreen(window, false);
                }
                break;

            default:
                break;
            }
        }
        else if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            w = event.window.data1;
            h = event.window.data2;
            framebuffer_size_callback(w, h);
        }
        else if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }
    }
}

void render() {

  
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
}

void clean() {

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void framebuffer_size_callback(int width, int height) {

    glViewport(0, 0, width, height);
}


