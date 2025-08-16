#include "simulation.hpp"

Simulation::Simulation()
    : fullscreen(false)
    , running(false)
    , w(0)
    , h(0)
    , particleVAO(0)
    , particleVBO(0)
    , springVAO(0)
    , springVBO(0)
    , uboMatrices(0)
    , window(nullptr)
    , context(nullptr)
    , basePath(nullptr)
    , deltaTime(0.0f)
    , lastFrame(0.0f)
    , camera(glm::vec3((cols - 1)* spacing * 0.5f, -(rows - 1) * spacing * 0.5f, 10.0f))
{
    // Create particles
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            glm::vec3 pos = glm::vec3(x * spacing, -y * spacing, 0.0f);
            particles.emplace_back(pos, 1.0f);
        }
    }

    float k_structural = 200.0f;
    float k_shear = 100.0f;
    float k_bend = 50.0f;

    float structural_damping = 10.0f;
    float shear_damping = 8.0f;
    float bend_damping = 5.0f;

    // Create springs
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int idx = y * cols + x;

            // Structural springs (horizontal and vertical)
            if (x < cols - 1) { // spring to right neighbor
                springs.emplace_back(&particles[idx], &particles[idx + 1], k_structural, structural_damping);
            }
            if (y < rows - 1) { // spring to neighbor below
                springs.emplace_back(&particles[idx], &particles[idx + cols], k_structural, structural_damping);
            }

            // Shear springs (diagonal)
            if (x < cols - 1 && y < rows - 1) { // diagonal down-right
                springs.emplace_back(&particles[idx], &particles[idx + cols + 1], k_shear, shear_damping);
            }
            if (x > 0 && y < rows - 1) { // diagonal down-left
                springs.emplace_back(&particles[idx], &particles[idx + cols - 1], k_shear, shear_damping);
            }

            // Bend springs (connect particles 2 steps apart)
            if (x < cols - 2) { // bend spring 2 steps to the right
                springs.emplace_back(&particles[idx], &particles[idx + 2], k_bend, bend_damping);
            }
            if (y < rows - 2) { // bend spring 2 steps down
                springs.emplace_back(&particles[idx], &particles[idx + 2 * cols], k_bend, bend_damping);
            }
        }
    }

    // Pin only the top row
    for (int x = 0; x < cols; ++x) {
        particles[x].pinned = true; 
    }

    // Pin only the top corners
   /* particles[0].pinned = true;
    particles[cols - 1].pinned = true;*/
}

void Simulation::run() {
    while (running) {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        deltaTime = std::min(deltaTime, 1.0f / 60.0f);

        processEvent();

        
        for (auto& s : springs) {
            s.applyForces();
        }

        for (auto& p : particles) {

            // Gravity
            p.addForce(glm::vec3(0.0f, -9.81f * p.mass, 0.0f));

            // Wind
            p.addForce(glm::vec3(-6.0f, 0.0f, 2.0f));
        }

        for (auto& p : particles) {
            p.updateVerlet(deltaTime);
        }

        for (int i = 0; i < 3; ++i) {
            for (auto& s : springs) {
                s.satisfyConstraint();
            }
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

    /*if (!SDL_SetWindowRelativeMouseMode(window, true)) {
        SDL_Log("SDL Mouse Mode: %s", SDL_GetError());
    }*/

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
    initSprings();
    initUBO();

    running = true;
    return true;
}

void Simulation::initUBO() {
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    glm::mat4 projection = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Simulation::initParticle() {
    
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}

void Simulation::initSprings() {
    glGenVertexArrays(1, &springVAO);
    glGenBuffers(1, &springVBO);

    glBindVertexArray(springVAO);
    glBindBuffer(GL_ARRAY_BUFFER, springVBO);
   
    glBufferData(GL_ARRAY_BUFFER, springs.size() * 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

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
    
    // draw particles
    std::vector<glm::vec3> positions;
    positions.reserve(particles.size());
    for (auto& p : particles) 
    {
        positions.emplace_back(p.position);
    }
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
    particleShader.setVec3("color", glm::vec3(1.0f, 0.9f, 0.3f));
    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, positions.size());

    // draw springs
    std::vector<glm::vec3> springPositions;
    springPositions.reserve(springs.size() * 2);
    for (auto& s : springs) {
        springPositions.emplace_back(s.p1->position);
        springPositions.emplace_back(s.p2->position);
    }
    glBindBuffer(GL_ARRAY_BUFFER, springVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, springPositions.size() * sizeof(glm::vec3), springPositions.data());
    particleShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
    glBindVertexArray(springVAO);
    glDrawArrays(GL_LINES, 0, springPositions.size());


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


