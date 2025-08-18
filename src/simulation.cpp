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
    , clothVAO(0)
    , clothVBO(0)
    , clothTexVBO(0)
    , flagVAO(0)
    , flagVBO(0)
    , flagTexVBO(0)
    , flagEBO(0)
    , clothTexture(0)
    , flagTexture(0)
    , flagNormVBO(0)
    , clothEBO(0)
    , uboMatrices(0)
    , window(nullptr)
    , context(nullptr)
    , basePath(nullptr)
    , mousePos(glm::vec2(0.0f))
    , deltaTime(0.0f)
    , lastFrame(0.0f)
    , tearRadius(0.1f)
    , leftMouseDown(false)
    , currentMode(SIMMODE::TEAR)
    , currentPinning(PINNINGMODE::TOP_ROW)
    , projectionMatrix(glm::mat4(0.0f))
    , isCameraActive(false)
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

    // Generate cloth texture coordinates
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            clothTexCoords.emplace_back(
                (float)x / (cols - 1),
                (float)y / (rows - 1)
            );
        }
    }

    // Generate flag texture coordinates
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            flagTexCoords.emplace_back(
                (float)x / (cols - 1),
                (float)y / (rows - 1)
            );
        }
    }

    // Generate indices for cloth mesh
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            int topLeft = y * cols + x;
            int topRight = y * cols + (x + 1);
            int bottomLeft = (y + 1) * cols + x;
            int bottomRight = (y + 1) * cols + (x + 1);

            clothIndices.emplace_back(topLeft);
            clothIndices.emplace_back(bottomLeft);
            clothIndices.emplace_back(topRight);

            clothIndices.emplace_back(topRight);
            clothIndices.emplace_back(bottomLeft);
            clothIndices.emplace_back(bottomRight);
        }
    }

    // Generate indices for flag mesh
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            int topLeft = y * cols + x;
            int topRight = y * cols + (x + 1);
            int bottomLeft = (y + 1) * cols + x;
            int bottomRight = (y + 1) * cols + (x + 1);

            flagIndices.emplace_back(topLeft);
            flagIndices.emplace_back(bottomLeft);
            flagIndices.emplace_back(topRight);

            flagIndices.emplace_back(topRight);
            flagIndices.emplace_back(bottomLeft);
            flagIndices.emplace_back(bottomRight);
        }
    }

    applyPinning();

    springActive.resize(springs.size(), true);
}

    void Simulation::applyPinning() {
        
        for (auto& p : particles) {
            p.pinned = false;
        }

        switch (currentPinning) {
        case PINNINGMODE::TOP_ROW:
            for (int x = 0; x < cols; ++x) {
                particles[x].pinned = true;
            }
            break;

        case PINNINGMODE::CORNERS:
            particles[0].pinned = true;
            particles[cols - 1].pinned = true;
            break;

        case PINNINGMODE::FLAG:
            for (int y = 0; y < rows; ++y) {
                particles[y * cols + 0].pinned = true;
            }
            /*particles[0].pinned = true;
            particles[(rows - 1) * cols].pinned = true;*/
            break;

        case PINNINGMODE::NONE:
            break;
        }
    }

void Simulation::run() {
    while (running) {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        deltaTime = std::min(deltaTime, 1.0f / 60.0f);

        processEvent();

        if (currentMode == SIMMODE::FLAG) {
            if (currentPinning != PINNINGMODE::FLAG) {
                currentPinning = PINNINGMODE::FLAG;
                applyPinning();
            }
        }

        handleMouseActivity();

        for (size_t i = 0; i < springs.size(); ++i) {
            if (springActive[i]) {
                springs[i].applyForces();
            }
        } 

        for (auto& p : particles) {

            // Gravity
            if (currentMode == SIMMODE::COLLISION) {
                // Gravity in -Z direction (down in rotated cloth space)
                p.addForce(glm::vec3(0.0f, 0.0f, -9.81f * p.mass));
            }
            else {
                // Normal gravity in -Y direction
                p.addForce(glm::vec3(0.0f, -9.81f * p.mass, 0.0f));
            }

            // Wind
            if (currentMode == SIMMODE::FLAG) {
                
                const glm::vec3 windDir = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
                float t = SDL_GetTicks() * 0.001f;
                float gust = 8.0f + 5.0f * std::sin(t * 1.5f) + 3.0f * std::sin(t * 0.5f + 1.0f);
                glm::vec3 lift = glm::vec3(0.0f, 0.2f, 0.0f);

                p.addForce(windDir * gust + lift);

                glm::vec3 v = (p.position - p.prevPosition) / glm::max(deltaTime, 1e-4f);
                p.addForce(-0.1f * v);


            }
        }

        for (auto& p : particles) {
            p.updateVerlet(deltaTime);
        }

        for (int i = 0; i < 5; ++i) {
            for (size_t j = 0; j < springs.size(); ++j) {
                if (springActive[j]) {
                    springs[j].satisfyConstraint();
                }
            }
        }     

        render();
    }
    clean();
}

void Simulation::reset() {
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int idx = y * cols + x;
            glm::vec3 originalPos = glm::vec3(x * spacing, -y * spacing, 0.0f);

            particles[idx].position = originalPos;
            particles[idx].prevPosition = originalPos;
            particles[idx].acceleration = glm::vec3(0.0f);
        }
    }

    currentPinning = PINNINGMODE::TOP_ROW; 
    applyPinning();

    std::fill(springActive.begin(), springActive.end(), true);

    switch (currentMode) {
    case SIMMODE::TEAR:
    {
        camera = { glm::vec3((cols - 1) * spacing * 0.5f, -(rows - 1) * spacing * 0.5f, 10.0f) };
        break;
    }
    case SIMMODE::FLAG:
    {
        camera = { glm::vec3(((cols - 1) * spacing * 0.5f) - 2.5f, (-(rows - 1) * spacing * 0.5f) - 7.0f, 25.0f) };
        break;
    }

    default:
        break;

    }
    

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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    #ifdef __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    #endif

    window = SDL_CreateWindow("ClothSimGL", WinWidth, WinHeight, WindowFlags);

    if (!window) {
        SDL_Log("SDL Window: %s", SDL_GetError());
        return false;
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
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
   

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

    clothShader = {
        (fs::path(basePath) / "assets" / "shaders" / "clothShader.vert").string().c_str(),
        (fs::path(basePath) / "assets" / "shaders" / "clothShader.frag").string().c_str()
    };

    flagShader = {
        (fs::path(basePath) / "assets" / "shaders" / "flagShader.vert").string().c_str(),
        (fs::path(basePath) / "assets" / "shaders" / "flagShader.frag").string().c_str()
    };

    poleShader = {
        (fs::path(basePath) / "assets" / "shaders" / "poleShader.vert").string().c_str(),
        (fs::path(basePath) / "assets" / "shaders" / "poleShader.frag").string().c_str()
    };

    clothTexture = loadTexture((fs::path(basePath) / "assets" / "textures" / "cloth.jpg").string().c_str());

    flagTexture = loadTexture((fs::path(basePath) / "assets" / "textures" / "flag.png").string().c_str());

    initParticle();
    initSprings();
    initClothMesh();
    initFlagMesh();
    initUBO();

    clothShader.use();
    clothShader.setInt("clothTexture", 0);

    flagShader.use();
    flagShader.setInt("material.diffuse", 0);

    running = true;
    return true;
}

void Simulation::initUBO() {
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    projectionMatrix = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
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

void Simulation::initClothMesh() {
    glGenVertexArrays(1, &clothVAO);
    glBindVertexArray(clothVAO);

    // Position VBO
    glGenBuffers(1, &clothVBO);
    glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoord VBO
    glGenBuffers(1, &clothTexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, clothTexVBO);
    glBufferData(GL_ARRAY_BUFFER, clothTexCoords.size() * sizeof(glm::vec2), clothTexCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    // EBO
    glGenBuffers(1, &clothEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, clothIndices.size() * sizeof(unsigned int), clothIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Simulation::initFlagMesh() {

    // flag
    glGenVertexArrays(1, &flagVAO);
    glBindVertexArray(flagVAO);

    // Position VBO
    glGenBuffers(1, &flagVBO);
    glBindBuffer(GL_ARRAY_BUFFER, flagVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoord VBO
    glGenBuffers(1, &flagTexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, flagTexVBO);
    glBufferData(GL_ARRAY_BUFFER, flagTexCoords.size() * sizeof(glm::vec2), flagTexCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    // Normal VBO
    glGenBuffers(1, &flagNormVBO);
    glBindBuffer(GL_ARRAY_BUFFER, flagNormVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);

    // EBO
    glGenBuffers(1, &flagEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flagEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flagIndices.size() * sizeof(unsigned int), flagIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // pole

    cylinder = generateCylinder(0.1f, 20.0f, 32);

    glGenVertexArrays(1, &poleVAO);
    glGenBuffers(1, &poleVBO);

    glBindVertexArray(poleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, poleVBO);
    glBufferData(GL_ARRAY_BUFFER, cylinder.size() * sizeof(PoleVertex), cylinder.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PoleVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PoleVertex), (void*)offsetof(PoleVertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}


glm::vec3 Simulation::screenToWorld(glm::vec2 screenPos, float depth) {
   
    glm::mat4 view = camera.GetViewMatrix();
    
    glm::vec4 viewport = glm::vec4(0, 0, w, h);

    glm::vec3 worldPos = glm::unProject(
        glm::vec3(screenPos.x, h - screenPos.y, depth),
        view,      
        projectionMatrix,
        viewport
    );

    return worldPos;
}

void Simulation::handleMouseActivity() {
    if (currentMode == SIMMODE::TEAR) {
        handleMouseTearing();
    }
}

void Simulation::handleMouseTearing() {
    if (leftMouseDown) {
        glm::vec3 nearPoint = screenToWorld(mousePos, 0.0f);
        glm::vec3 farPoint = screenToWorld(mousePos, 1.0f);
        glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

        Particle* targetParticle = findClosestParticleToRay(nearPoint, rayDir);

        if (targetParticle != nullptr) {
            tearSpringsAroundPoint(targetParticle->position, tearRadius);
        }
    }
}

void Simulation::tearSpringsAroundPoint(glm::vec3 worldPos, float radius) {
    int tornCount = 0; 

    for (size_t i = 0; i < springs.size(); ++i) {
        if (!springActive[i]) continue;

        glm::vec3 p1 = springs[i].p1->position;
        glm::vec3 p2 = springs[i].p2->position;

        float dist1 = glm::length(worldPos - p1);
        float dist2 = glm::length(worldPos - p2);

        if (dist1 < radius || dist2 < radius) {
            springActive[i] = false;
            tornCount++;
            continue;
        }

        glm::vec3 springVec = p2 - p1;
        float springLength = glm::length(springVec);

        if (springLength > 0) {
            glm::vec3 springDir = springVec / springLength;
            glm::vec3 toTearPoint = worldPos - p1;

            float t = glm::clamp(glm::dot(toTearPoint, springDir), 0.0f, springLength);
            glm::vec3 closestPoint = p1 + springDir * t;

            float distanceToTear = glm::length(worldPos - closestPoint);

            if (distanceToTear < radius) {
                springActive[i] = false;
                tornCount++;
            }
        }
    }

}

Particle* Simulation::findClosestParticleToRay(glm::vec3 rayOrigin, glm::vec3 rayDir) {
    float min_dist_sq = std::numeric_limits<float>::max();
    Particle* closest_particle = nullptr;

    if (particles.empty()) {
        return nullptr;
    }

    for (auto& p : particles) {
       
        glm::vec3 vec_to_particle = p.position - rayOrigin;
        
        glm::vec3 closest_point_on_ray = rayOrigin + glm::dot(vec_to_particle, rayDir) * rayDir;
       
        glm::vec3 vec = p.position - closest_point_on_ray;
        float dist_sq = glm::dot(vec, vec);

        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest_particle = &p;
        }
    }
    
    if (closest_particle && sqrt(min_dist_sq) < tearRadius * 2.0f) {
        return closest_particle;
    }

    return nullptr;
}

void Simulation::processEvent() {
    SDL_Event event;
    SDL_zero(event);

    const bool* keyState = SDL_GetKeyboardState(nullptr);

    if (isCameraActive) {
        if (keyState[SDL_SCANCODE_W]) {
            camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (keyState[SDL_SCANCODE_S]) {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (keyState[SDL_SCANCODE_A]) {
            camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (keyState[SDL_SCANCODE_D]) {
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }
    }

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
            case SDLK_R:
                reset();
                break;
            case SDLK_E:
                currentMode = static_cast<SIMMODE>((static_cast<int>(currentMode) + 1) % static_cast<int>(SIMMODE::LAST));
                reset();
                break;
            case SDLK_P:   
                currentPinning = static_cast<PINNINGMODE>((static_cast<int>(currentPinning) + 1) % static_cast<int>(PINNINGMODE::LAST));
                applyPinning();
                break;
            case SDLK_SPACE:
                isCameraActive = !isCameraActive;
                SDL_SetWindowRelativeMouseMode(window, isCameraActive);
                if (isCameraActive) leftMouseDown = false;
                break;
            default:
                break;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                leftMouseDown = true;
                mousePos = glm::vec2(event.button.x, event.button.y);
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                leftMouseDown = false;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            if (isCameraActive) {
                camera.ProcessMouseMovement(event.motion.xrel, -event.motion.yrel);
            }
            else {
                mousePos = glm::vec2(event.motion.x, event.motion.y);
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

std::vector<glm::vec3> Simulation::computeNormals() {
    std::vector<glm::vec3> normals(particles.size(), glm::vec3(0.0f));
    // Accumulate per-triangle normals
    for (size_t i = 0; i < flagIndices.size(); i += 3) {
        unsigned int ia = flagIndices[i + 0];
        unsigned int ib = flagIndices[i + 1];
        unsigned int ic = flagIndices[i + 2];
        const glm::vec3& a = particles[ia].position;
        const glm::vec3& b = particles[ib].position;
        const glm::vec3& c = particles[ic].position;
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        normals[ia] += n;
        normals[ib] += n;
        normals[ic] += n;
    }
    // Normalize
    for (auto& n : normals) {
        float len = glm::length(n);
        if (len > 1e-6f) n /= len;
        else n = glm::vec3(0, 0, 1);
    }
    return normals;
}

std::vector<PoleVertex> Simulation::generateCylinder(float radius, float height, int slices) {
    constexpr float PI = 3.14159265359f;
    std::vector<PoleVertex> vertices;

    for (int i = 0; i < slices; i++) {
        float theta = 2.0f * PI * i / slices;
        float nextTheta = 2.0f * PI * (i + 1) / slices;

        float x0 = radius * cos(theta), z0 = radius * sin(theta);
        float x1 = radius * cos(nextTheta), z1 = radius * sin(nextTheta);

        float y0 = 0.0f, y1 = height;

        glm::vec3 n0 = glm::normalize(glm::vec3(cos(theta), 0.0f, sin(theta)));
        glm::vec3 n1 = glm::normalize(glm::vec3(cos(nextTheta), 0.0f, sin(nextTheta)));

        // First triangle
        vertices.push_back({ glm::vec3(x0, y0, z0), n0 });
        vertices.push_back({ glm::vec3(x1, y0, z1), n1 });
        vertices.push_back({ glm::vec3(x1, y1, z1), n1 });

        // Second triangle
        vertices.push_back({ glm::vec3(x0, y0, z0), n0 });
        vertices.push_back({ glm::vec3(x1, y1, z1), n1 });
        vertices.push_back({ glm::vec3(x0, y1, z0), n0 });
    }

    return vertices;
}


void Simulation::render() {


    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // draw particles
   /* std::vector<glm::vec3> positions;
    positions.reserve(particles.size());
    for (auto& p : particles) 
    {
        positions.emplace_back(p.position);
    }
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
    particleShader.setVec3("color", glm::vec3(1.0f, 0.9f, 0.3f));
    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, positions.size());*/

    
    switch (currentMode) {

        // draw springs
        case SIMMODE::TEAR:
        {
            particleShader.use();
            std::vector<glm::vec3> activeSpringPositions;
            activeSpringPositions.reserve(springs.size() * 2);

            for (size_t i = 0; i < springs.size(); ++i) {
                if (springActive[i]) {
                    activeSpringPositions.emplace_back(springs[i].p1->position);
                    activeSpringPositions.emplace_back(springs[i].p2->position);
                }
            }

            if (!activeSpringPositions.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, springVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, activeSpringPositions.size() * sizeof(glm::vec3), activeSpringPositions.data());
                particleShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
                glBindVertexArray(springVAO);
                glDrawArrays(GL_LINES, 0, activeSpringPositions.size());
            }
            break;
        }
        // draw textures
        case SIMMODE::COLLISION:
        {
            clothShader.use();
            glm::mat4 clothModel = glm::mat4(1.0f);
            clothShader.setMat4("model", clothModel);

            std::vector<glm::vec3> positions;
            positions.reserve(particles.size());
            for (auto& p : particles) {
                positions.emplace_back(p.position);
            }
            glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

            glBindVertexArray(clothVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, clothTexture);
            glDrawElements(GL_TRIANGLES, clothIndices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            break;
        }
        case SIMMODE::FLAG:
        {

            flagShader.use();
            flagShader.setVec3("light.direction", 1.2f, 1.0f, 5.0f);
            flagShader.setVec3("viewPos", camera.Position);

            // light properties
            flagShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
            flagShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
            flagShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            // material properties
            flagShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
            flagShader.setFloat("material.shininess", 32.0f);

            glm::mat4 flagModel = glm::mat4(1.0f);
            flagModel = glm::translate(flagModel, glm::vec3(0.0f, 0.0f, 0.0f));
            flagShader.setMat4("model", flagModel);

            std::vector<glm::vec3> positions;
            positions.reserve(particles.size());
            for (auto& p : particles) {
                positions.emplace_back(p.position);
            }
            glBindBuffer(GL_ARRAY_BUFFER, flagVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

            // Update normals
            std::vector<glm::vec3> normals = computeNormals();
            glBindBuffer(GL_ARRAY_BUFFER, flagNormVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), normals.data());

            glBindVertexArray(flagVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, flagTexture);
            
            glDrawElements(GL_TRIANGLES, flagIndices.size(), GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -20.0f, 0.0f));

            poleShader.use();
            poleShader.setMat4("model", model);

            poleShader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
            poleShader.setVec3("viewPos", camera.Position);
            poleShader.setVec3("lightColor", glm::vec3(1.0f));
            poleShader.setVec3("objectColor", glm::vec3(0.8f, 0.8f, 0.8f));

            glBindVertexArray(poleVAO);
            glDrawArrays(GL_TRIANGLES, 0, cylinder.size());
            glBindVertexArray(0);

            break;
        }

    }

    SDL_GL_SwapWindow(window);
}

void Simulation::clean() {
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    glDeleteVertexArrays(1, &springVAO);
    glDeleteBuffers(1, &springVBO);
    glDeleteVertexArrays(1, &clothVAO);
    glDeleteBuffers(1, &clothVBO);
    glDeleteBuffers(1, &clothTexVBO);
    glDeleteBuffers(1, &clothEBO);
    glDeleteTextures(1, &clothTexture);
    glDeleteVertexArrays(1, &flagVAO);
    glDeleteBuffers(1, &flagVBO);
    glDeleteBuffers(1, &flagTexVBO);
    glDeleteBuffers(1, &flagEBO);
    glDeleteBuffers(1, &flagNormVBO);
    glDeleteTextures(1, &flagTexture);
    glDeleteVertexArrays(1, &poleVAO);
    glDeleteBuffers(1, &poleVBO);
    particleShader.clean();
    clothShader.clean();
    poleShader.clean();
    flagShader.clean();
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Simulation::framebuffer_size_callback(int width, int height) {
    glViewport(0, 0, width, height);

    projectionMatrix = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned int Simulation::loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    SDL_Surface* surface = IMG_Load(path);

    if (surface)
    {

        int nrComponents = SDL_BYTESPERPIXEL(surface->format);
        
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SDL_DestroySurface(surface);
    }
    else
    {
        SDL_Log("Failed to load texture: %s\n", SDL_GetError());
    }

    return textureID;
}


