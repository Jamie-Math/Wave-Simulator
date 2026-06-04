#pragma once
#include "Grid.h"

class Simulator {
public:
    Simulator(Grid& grid, float c, float dt, float dx, float damping);
    void step();

private:
    Grid& grid;
    float k;       // precomputed (c*dt/dx)^2
    float damping; // ~0.999 — bleeds energy slowly

    // zero out all six boundary faces — keeps waves from wrapping
    void applyBoundaries();
};