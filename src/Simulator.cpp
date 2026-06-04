#include "Simulator.h"

#include <iostream>

Simulator::Simulator(Grid& g, float c, float dt, float dx, float damping)
    : grid(g), damping(damping) {
  k = (c * dt / dx) * (c * dt / dx);

  // CFL check — if k > 1/3 the sim will explode
  // 1/3 comes from 1/sqrt(3) squared for 3D stability
  if (k > 1.0f / 3.0f) {
    std::cout << "WARNING: CFL violated. Reduce dt or increase dx.\n";
  }
}

void Simulator::step() {
  int w = grid.width;
  int h = grid.height;
  int d = grid.depth;

  // TODO: triple nested loop — x: 1..w-2, y: 1..h-2, z: 1..d-2
  // skip solid cells: if (grid.solid[grid.index(x,y,z)]) continue;
  //
  // read the six neighbours from grid.current:
  //   float cur   = grid.current[grid.index(x,   y,   z  )];
  //   float left  = grid.current[grid.index(x-1, y,   z  )];
  //   float right = grid.current[grid.index(x+1, y,   z  )];
  //   float down  = grid.current[grid.index(x,   y-1, z  )];
  //   float up    = grid.current[grid.index(x,   y+1, z  )];
  //   float back  = grid.current[grid.index(x,   y,   z-1)];
  //   float front = grid.current[grid.index(x,   y,   z+1)];
  //
  // apply the wave stencil:
  //   float next = 2*cur - grid.previous[grid.index(x,y,z)]
  //              + k * (left + right + up + down + front + back - 6*cur);
  //   next *= damping;
  //
  // write into grid.previous (safe — we're still reading grid.current):
  //   grid.previous[grid.index(x,y,z)] = next;

  grid.swap();  // previous now holds the new state, swap makes it current

  applyBoundaries();
}

void Simulator::applyBoundaries() {
  int w = grid.width;
  int h = grid.height;
  int d = grid.depth;

  // TODO: zero the six boundary faces
  // face x=0 and x=w-1: loop y,z and set current[index(0,y,z)] = 0 etc.
  // face y=0 and y=h-1: loop x,z
  // face z=0 and z=d-1: loop x,y
}