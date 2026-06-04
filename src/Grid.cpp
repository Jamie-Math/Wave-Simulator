#include "Grid.h"

Grid::Grid(int w, int h, int d) : width(w), height(h), depth(d) {
  int total = w * h * d;

  // TODO: resize current, previous to total, fill with 0.0f
  // TODO: resize solid to total, fill with false
}

int Grid::index(int x, int y, int z) const {
  // TODO: return z * width * height + y * width + x
  return 0;
}

void Grid::swap() {
  // TODO: std::swap(current, previous)
}

void Grid::addSolidBox(int x0, int y0, int z0, int x1, int y1, int z1) {
  // TODO: triple nested loop over x0..x1, y0..y1, z0..z1
  // solid[index(x,y,z)] = true for each cell
}

void Grid::addImpulse(int x, int y, int z, float value) {
  // TODO: bounds check — return if out of range
  // current[index(x,y,z)] = value
}