#include "simulation.hpp"

Simulation::Simulation() : fullscreen(false)
    ,running(false)
    ,w(0)
    ,h(0)
    ,particleVAO(0)
    ,particleVBO(0)
    ,uboMatrices(0)
    ,window(nullptr)
    ,context(nullptr)
    ,basePath(nullptr)
    ,deltaTime(0.0f)
    ,lastFrame(0.0f)
    ,camera(glm::vec3((cols - 1) * spacing * 0.5f, -(rows - 1) * spacing * 0.5f, 3.0f))
{ 
    
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            glm::vec3 pos = glm::vec3(x * spacing, -y * spacing, 0.0f);
            particles.emplace_back(pos);
            
        }
    }


    for (auto& p : particles) {
        p.pinned = true;
    }
    
}

void Simulation::run() {

    while (running) {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        deltaTime = std::min(deltaTime, (float)(1.0f / 60));

        processEvent();

        for (auto& p : particles) {
            p.addForce(glm::vec3(0.0f, -1.00f, 0.0f));
            p.updateVerlet(deltaTime);
        }

        render();
    }
    clean();
}

bool Simulation::init() {
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

    glEnable(GL_PROGRAM_POINT_SIZE);

    basePath = SDL_GetBasePath();

    if (!basePath) {
        SDL_Log("Error getting base path: %s", SDL_GetError());
        return false;
    }

    SDL_GetWindowSize(window, &w, &h);
    framebuffer_size_callback(w, h);

    particleShader = { 
        (fs::path(basePath) / "assets" / "shaders" / "particleShader.vert").string().c_str(),
        (fs::path(basePath) / "assets" / "shaders" / "particleShader.frag").string().c_str()
    };

    initParticle();

    initUBO();

    running = true;
    return true;
}

void Simulation::initUBO() {
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    glm::mat4 projection = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Simulation::initParticle() {
    int numParticles = rows * cols;

    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, numParticles * sizeof(glm::vec3), nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}

void Simulation::processEvent() {
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

void Simulation::render() {


    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    particleShader.use();
    
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    std::vector<glm::vec3> positions;
    for (auto& p : particles) positions.push_back(p.position);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, positions.size());

    SDL_GL_SwapWindow(window);
}

void Simulation::clean() {
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    particleShader.clean();
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Simulation::framebuffer_size_callback(int width, int height) {

    glViewport(0, 0, width, height);
}


