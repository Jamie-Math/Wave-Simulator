#include "Grid.hpp"

Grid::Grid(int w, int h, int l) : width(w), height(h), length(l) {
  {
    solid = std::vector<bool>(w * l * h, 0);
    previous = std::vector<float>(w * l * h, 0);
    current = std::vector<float>(w * l * h, 0);
  }
}

int Grid::index(int x, int y, int z) const {
  return x + (y * width) + z * (width * height);
}

void Grid::swap() { std::swap(current, previous); }

void Grid::addSolidBox(int x0, int y0, int z0, int x1, int y1, int z1) {
  for (int x = x0; x <= x1; x++) {
    for (int y = y0; y <= y1; y++) {
      for (int z = z0; z <= z1; z++) {
        auto position = index(x, y, z);
        solid[position] = 1;
        current[position] = 0.0f;
        previous[position] = 0.0f;
      }
    }
  }
}

void Grid::addImpulse(int x, int y, int z, float value) {
  if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= length)
    return;
  current[index(x, y, z)] = value;
}