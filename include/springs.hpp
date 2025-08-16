#pragma once
#include "particle.hpp"

class Spring {
public:  
    Particle* p1;
    Particle* p2;

private:
    float stiffness;
    float restLength;
    float damping;

public:
    Spring(Particle* particleA, Particle* particleB, float k, float dampingCoeff = 0.1f);
    void applyForces();
    void satisfyConstraint();
};