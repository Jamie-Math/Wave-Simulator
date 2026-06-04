#include "app.hpp"

#include <SDL2/SDL.h>

App::App()
    : grid(50, 50, 50),                           // small grid — good for MVP
      simulator(grid, 1.0f, 0.2f, 1.0f, 0.999f),  // c=1, dt=0.2, dx=1, damping
      renderer(800, 800),
      running(true) {
  // floor

  // start a wave — big impulse near one edge
  grid.addImpulse(15, 15, 2, 2.0f);
}

void App::run() {
  float time = 0.0f;
  while (running) {
    /* code */
    // ===> ADD THIS LINE TO DRIVER THE WATER CONTINUOUSLY <===
    // This constantly forces a specific coordinate to bob up and down like a
    // motor
    handleEvents();
    float waveSourceValue = std::sin(time * 2.0f) * 3.0f;
    if ((int)(time * 11) % 13 == 0) {
      grid.addImpulse(grid.width / 2, grid.height - 30 + 10, grid.length / 2,
                      3.0f);
    }
    simulator.step();
    renderer.draw(grid);
    renderer.present();

    time += 0.1f;  // Advance time for the sine wave
    SDL_Delay(33);
  }
  //-- rough 60fps cap,
  // replace with proper timing later
}
#include <iostream>
void App::handleEvents() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) running = false;

    // press space to fire a new impulse at the centre
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
      grid.addImpulse(15, 15, 2, 2.0f);
  }
}