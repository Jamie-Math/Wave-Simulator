#pragma once
#include "include/Grid.hpp"

class Simulator {
 public:
  Simulator(Grid& grid, float c, float dt, float dx, float damping);
  void step();

 private:
  Grid& grid;
  // Transfer constant
  const float k;
  const float damping = 0.9999f;

  // zero out all six boundary faces — keeps waves from wrapping
  void applyBoundaries();
};