#include <particle.hpp>

Particle::Particle(const glm::vec3& startPos, float m) 
    : position(startPos)
    , prevPosition(startPos)
    , acceleration(0.0f)
    , mass(m)
    , pinned(false)
{
}

void Particle::addForce(const glm::vec3& force) {
    acceleration += force / mass;
}

void Particle::updateVerlet(float dt) {
    if (pinned) return;
    glm::vec3 temp = position;
    position += (position - prevPosition) + acceleration * (dt * dt);
    prevPosition = temp;
    acceleration = glm::vec3(0.0f);
}

void Particle::pinTo(const glm::vec3& pos) {
    pinned = true;
    position = pos;
    prevPosition = pos;
}