#pragma once
#include <glm/glm.hpp>

class Particle {
public:
    glm::vec3 position;
    glm::vec3 prevPosition;
    glm::vec3 acceleration;
    float mass;
    bool pinned;

    Particle(const glm::vec3& startPos, float m = 1.0f);

    void addForce(const glm::vec3& force);

    void updateVerlet(float dt);

    void pinTo(const glm::vec3& pos);
};
