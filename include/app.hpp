#pragma once
#include "Grid.hpp"
#include "Renderer.hpp"
#include "Simulator.hpp"

class App {
 public:
  App();
  void run();

 private:
  Grid grid;
  Simulator simulator;
  Renderer renderer;
  bool running;

  void handleEvents();
};