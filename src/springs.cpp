#include "springs.hpp"

Spring::Spring(Particle* particleA, Particle* particleB, float k, float dampingCoeff)
    : p1(particleA)
    , p2(particleB)
    , stiffness(k)
    , damping(dampingCoeff)
{
    restLength = glm::length(p1->position - p2->position);
}

void Spring::applyForces() {
    glm::vec3 delta = p2->position - p1->position;
    float currentLength = glm::length(delta);

    if (currentLength == 0.0f) return;

    glm::vec3 direction = delta / currentLength;

    float displacement = currentLength - restLength;
    float stretchRatio = currentLength / restLength;

    // Apply increasingly strong force as stretch increases
    float forceMultiplier = 1.0f;
    if (stretchRatio > 1.1f) {

        forceMultiplier = pow(stretchRatio, 3.0f);
    }

    glm::vec3 springForce = stiffness * displacement * forceMultiplier * direction;

    // Add damping
    glm::vec3 p1Velocity = p1->position - p1->prevPosition;
    glm::vec3 p2Velocity = p2->position - p2->prevPosition;
    glm::vec3 relativeVelocity = p2Velocity - p1Velocity;

    float dampingMagnitude = glm::dot(relativeVelocity, direction);
    glm::vec3 dampingForce = damping * dampingMagnitude * direction;

    glm::vec3 totalForce = springForce + dampingForce;

    p1->addForce(totalForce);
    p2->addForce(-totalForce);
}

void Spring::satisfyConstraint() {
    
    glm::vec3 delta = p2->position - p1->position;
    float currentLength = glm::length(delta);

    if (currentLength == 0.0f) return;

    float maxLength = restLength * 1.2f;
    if (currentLength > maxLength) {
        float excess = currentLength - maxLength;
        glm::vec3 direction = delta / currentLength;
        glm::vec3 correction = direction * excess * 0.5f;

        if (!p1->pinned && !p2->pinned) {
            p1->position += correction;
            p2->position -= correction;
        }
        else if (p1->pinned && !p2->pinned) {
            p2->position -= correction * 2.0f;
        }
        else if (!p1->pinned && p2->pinned) {
            p1->position += correction * 2.0f;
        }
    }
}