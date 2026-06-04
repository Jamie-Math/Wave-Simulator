#include "Renderer.h"

#include <cmath>
#include <iostream>

// -------------------------------------------------------
// SHADER SOURCE
// these are the two programs that run on the GPU
// -------------------------------------------------------

// vertex shader: runs once per vertex
// gl_Position is where on screen this vertex lands
// it receives the cell's world position and colour as instance data
const char* vertSrc = R"(
#version 330 core

layout(location = 0) in vec3 aPos;       // vertex of the unit cube
layout(location = 1) in vec3 iPos;       // instance: cell world position
layout(location = 2) in vec3 iColour;    // instance: cell colour

out vec3 fragColour;
out vec3 fragNormal; // we'll fake a normal from vertex position for cell shading

uniform mat4 uMVP; // model-view-projection matrix — set from CPU each frame

void main() {
    // place the unit cube at the cell's world position
    vec3 worldPos = aPos + iPos;
    gl_Position = uMVP * vec4(worldPos, 1.0);

    fragColour = iColour;
    // fake normal: use the vertex position on the cube to approximate face normal
    // good enough for cell shading without a full normal buffer
    fragNormal = normalize(aPos);
}
)";

// fragment shader: runs once per pixel
// cell shading = quantise the lighting into discrete bands (e.g. 3 levels)
// this gives the hard-edged cartoon look
const char* fragSrc = R"(
#version 330 core

in vec3 fragColour;
in vec3 fragNormal;

out vec4 outColour;

uniform vec3 uLightDir; // direction light is coming from — set from CPU

void main() {
    // diffuse lighting: how much does this face point toward the light
    float diff = max(dot(normalize(fragNormal), normalize(uLightDir)), 0.0);

    // TODO: quantise diff into cel shading bands
    // example three-band quantisation:
    //   if (diff > 0.8) diff = 1.0;
    //   else if (diff > 0.4) diff = 0.6;
    //   else diff = 0.2;
    // this is what creates the hard cartoon shading steps

    // apply lighting to colour
    vec3 lit = fragColour * diff;
    outColour = vec4(lit, 1.0);
}
)";

Renderer::Renderer(int w, int h) {
  // TODO: SDL_Init(SDL_INIT_VIDEO)
  // TODO: SDL_GL_SetAttribute for OpenGL 3.3 core profile
  //   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  //   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  //   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
  //   SDL_GL_CONTEXT_PROFILE_CORE);
  // TODO: SDL_CreateWindow with SDL_WINDOW_OPENGL flag
  // TODO: SDL_GL_CreateContext(window)
  // TODO: glewInit()
  // TODO: glEnable(GL_DEPTH_TEST) — needed for correct 3D rendering
  // TODO: glViewport(0, 0, w, h)

  shaderProgram = compileShaders();
  setupCubeMesh();

  // create the instance VBO — we'll resize it each frame
  // TODO: glGenBuffers(1, &instanceVBO)
}

Renderer::~Renderer() {
  // TODO: glDeleteBuffers, glDeleteVertexArrays
  // TODO: SDL_GL_DeleteContext, SDL_DestroyWindow, SDL_Quit
}

GLuint Renderer::compileShaders() {
  // TODO: standard OpenGL shader compile steps:
  // 1. glCreateShader(GL_VERTEX_SHADER), glShaderSource, glCompileShader
  // 2. same for GL_FRAGMENT_SHADER
  // 3. glCreateProgram, glAttachShader x2, glLinkProgram
  // 4. check for errors with glGetShaderiv(shader, GL_COMPILE_STATUS, ...)
  //    print glGetShaderInfoLog if it failed
  // 5. glDeleteShader both after linking
  // return the program handle
  return 0;
}

void Renderer::setupCubeMesh() {
  // TODO: define 36 vertices for a unit cube (side 0.9 to leave small gaps
  // between cells) each vertex is a vec3. 12 triangles * 3 vertices = 36 you
  // can find a standard cube vertex list by searching "OpenGL unit cube
  // vertices" or generate it — 6 faces, 2 triangles each, 3 vertices each
  //
  // then:
  // glGenVertexArrays(1, &cubeVAO)
  // glBindVertexArray(cubeVAO)
  // glGenBuffers(1, &cubeVBO)
  // glBindBuffer(GL_ARRAY_BUFFER, cubeVBO)
  // glBufferData with the vertex data
  // glVertexAttribPointer(0, 3, GL_FLOAT, ...) — location 0 = vertex pos
  // glEnableVertexAttribArray(0)
}

void Renderer::amplitudeToColour(float v, float& r, float& g, float& b) {
  // TODO: map amplitude to colour
  // simple version:
  //   positive values: r = v/maxAmp, g = 0, b = 0  (red)
  //   negative values: r = 0, g = 0, b = -v/maxAmp (blue)
  // clamp all channels to 0..1
  float maxAmp = 1.0f;  // tune this to your impulse strength
  if (v > 0) {
    r = v / maxAmp;
    g = 0;
    b = 0;
  } else {
    r = 0;
    g = 0;
    b = -v / maxAmp;
  }
}

void Renderer::draw(const Grid& grid) {
  // clear screen
  // TODO: glClearColor(0.05f, 0.05f, 0.05f, 1.0f) — dark background
  // TODO: glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

  // build instance list — one entry per visible cell
  std::vector<CellInstance> instances;
  instances.reserve(grid.width * grid.height * grid.depth / 4);

  for (int z = 0; z < grid.depth; z++)
    for (int y = 0; y < grid.height; y++)
      for (int x = 0; x < grid.width; x++) {
        int i = grid.index(x, y, z);

        // solid cells: render as grey obstacle
        if (grid.solid[i]) {
          instances.push_back({(float)x, (float)y, (float)z, 0.4f, 0.4f, 0.4f});
          continue;
        }

        float val = grid.current[i];

        // skip near-zero cells — they're invisible and waste draw calls
        if (std::abs(val) < threshold) continue;

        float r, g, b;
        amplitudeToColour(val, r, g, b);
        instances.push_back({(float)x, (float)y, (float)z, r, g, b});
      }

  if (instances.empty()) return;

  // upload instance data to GPU
  // TODO: glBindBuffer(GL_ARRAY_BUFFER, instanceVBO)
  // TODO: glBufferData(GL_ARRAY_BUFFER, instances.size()*sizeof(CellInstance),
  //                    instances.data(), GL_DYNAMIC_DRAW)

  // set up instance attribute pointers on the VAO
  // TODO: glBindVertexArray(cubeVAO)
  // location 1 = iPos (x,y,z), offset 0 in CellInstance
  // TODO: glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance),
  // (void*)0)
  // TODO: glEnableVertexAttribArray(1)
  // TODO: glVertexAttribDivisor(1, 1)  — advance once per instance, not per
  // vertex location 2 = iColour (r,g,b), offset 12 bytes in CellInstance
  // TODO: glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance),
  // (void*)12)
  // TODO: glEnableVertexAttribArray(2)
  // TODO: glVertexAttribDivisor(2, 1)

  // set shader uniforms
  // TODO: glUseProgram(shaderProgram)
  //
  // MVP matrix — for now a simple perspective + view + identity model
  // you need a maths library for this — glm is the standard choice
  // add to your build: #include <glm/glm.hpp> and
  // <glm/gtc/matrix_transform.hpp>
  //
  // glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f,
  // 500.0f); glm::mat4 view = glm::lookAt(
  //     glm::vec3(w/2, h*1.5f, d*2),  // camera position — above and behind the
  //     grid glm::vec3(w/2, h/2,    d/2),  // look at centre of grid
  //     glm::vec3(0, 1, 0)            // up vector
  // );
  // glm::mat4 mvp = proj * view;
  // GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
  // glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
  //
  // light direction uniform:
  // GLint lightLoc = glGetUniformLocation(shaderProgram, "uLightDir");
  // glUniform3f(lightLoc, 1.0f, 2.0f, 1.0f); // light from upper right

  // draw all instances in one call
  // TODO: glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instances.size())
}

void Renderer::present() {
  // TODO: SDL_GL_SwapWindow(window)
}