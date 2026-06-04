#pragma once
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <vector>

#include "Simulator.hpp"

// one entry per visible cell passed to the GPU
struct CellInstance {
  float x, y, z;
  float r, g, b, a;
};

class Renderer {
 public:
  Renderer(int windowW, int windowH);
  ~Renderer();
  std::vector<bool> nearActivity;

  // build instance list from grid and draw all cells
  void draw(const Grid& grid);

  // swap SDL window buffers
  void present();

 private:
  bool hasActivityNearby(const Grid& grid, int cx, int cy, int cz, int radius);
  GLuint heightTexture = 0;
  bool textureInitialized = false;
  SDL_Window* window;
  SDL_GLContext glContext;

  GLuint cubeVAO, cubeVBO;  // unit cube mesh (shared)
  GLuint instanceVBO;       // per-cell position + colour data
  GLuint shaderProgram;

  // compile and link the vertex + fragment shaders
  // TODO: implement this — see shader source strings below
  GLuint compileShaders();

  // upload the unit cube vertices to cubeVBO
  // a unit cube is just 36 vertices (12 triangles, no index buffer for
  // simplicity)
  // TODO: generate the 36 vertices for a cube centred at origin, side length 1
  void setupCubeMesh();

  // map a float amplitude to an RGB colour
  // TODO: negative = blue, positive = red, near zero = dark/transparent
  // for cell shading: cells near zero can just be skipped entirely (don't add
  // to instance list)
  void amplitudeToColour(float value, float& r, float& g, float& b);

  // threshold below which cells are invisible — keeps the scene readable
  // start with 0.05f and tune visually
  float threshold = 0.05f;
};