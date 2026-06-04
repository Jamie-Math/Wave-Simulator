#include "include/Simulator.hpp"

#include <iostream>

Simulator::Simulator(Grid& g, float c, float dt, float dx, float damping)
    : grid(g), damping(damping), k(c * dt / dx) * (c * dt / dx) {
  // CFL check — if k > 1/3 the sim will explode
  // 1/3 comes from 1/sqrt(3) squared for 3D stability
  if (k > 1.0f / 3.0f) {
    std::cout << "WARNING: CFL violated. Reduce dt or increase dx.\n";
  }
}

void Simulator::step() {
  int w = grid.width;
  int h = grid.height;
  int l = grid.length;

  for (int x = 1; x < w - 1; x++) {
    for (int y = 1; y < h - 1; y++) {
      for (int z = 1; z < l - 1; z++) {
        if (grid.solid[grid.index(x, y, z)])
          continue;
        else {
          float cur = grid.current[grid.index(x, y, z)];
          float left = grid.current[grid.index(x - 1, y, z)];
          float right = grid.current[grid.index(x + 1, y, z)];
          float down = grid.current[grid.index(x, y - 1, z)];
          float up = grid.current[grid.index(x, y + 1, z)];
          float back = grid.current[grid.index(x, y, z - 1)];
          float front = grid.current[grid.index(x, y, z + 1)];
          float next = 2 * cur - grid.previous[grid.index(x, y, z)] +
                       k * (left + right + up + down + front + back - 6 * cur);
          next *= damping;
          grid.previous[grid.index(x, y, z)] = next;
        }
      }
    }
  }

  grid.swap();  // previous now holds the new state, swap makes it current

  applyBoundaries();
}

void Simulator::applyBoundaries() {
  // left and right faces — loop y and z
  for (int y = 0; y < grid.height; y++)
    for (int z = 0; z < grid.length; z++) {
      grid.current[grid.index(0, y, z)] = 0.0f;
      grid.current[grid.index(grid.width - 1, y, z)] = 0.0f;
    }

  // top and bottom faces — loop x and z
  for (int x = 0; x < grid.width; x++)
    for (int z = 0; z < grid.length; z++) {
      grid.current[grid.index(x, 0, z)] = 0.0f;
      grid.current[grid.index(x, grid.height - 1, z)] = 0.0f;
    }

  // front and back faces — loop x and y
  for (int x = 0; x < grid.width; x++)
    for (int y = 0; y < grid.height; y++) {
      grid.current[grid.index(x, y, 0)] = 0.0f;
      grid.current[grid.index(x, y, grid.length - 1)] = 0.0f;
    }
}