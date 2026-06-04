#include "Renderer.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// -------------------------------------------------------
// SHADER SOURCE
// these are the two programs that run on the GPU
// -------------------------------------------------------

// vertex shader: runs once per vertex
// gl_Position is where on screen this vertex lands
// it receives the cell's world position and colour as instance data

// fragment shader: runs once per pixel
// cell shading = quantise the lighting into discrete bands (e.g. 3 levels)
// this gives the hard-edged cartoon look
const char* vertSrc = R"(
#version 330 core

layout(location = 0) in vec3 aPos;       
layout(location = 1) in vec3 iPos;       
layout(location = 2) in vec4 iColour;    // Changed to vec4 for RGBA

out vec4 fragColour;                     // Changed to vec4
out vec3 fragNormal; 

uniform mat4 uMVP; 

void main() {
    vec3 worldPos = aPos + iPos;
    gl_Position = uMVP * vec4(worldPos, 1.0);

    fragColour = iColour; 
    fragNormal = normalize(aPos);
}
)";
const char* fragSrc = R"(
#version 330 core

in vec4 fragColour;
in vec3 fragNormal;

out vec4 outColour;

uniform vec3 uLightDir;

void main() {
    // 1. Calculate the core lighting angle
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(uLightDir);
    float NdotL = dot(normal, lightDir);
    
    // 2. CEL SHADING: Quantize the light into 3 hard, flat bands
    float celLight;
    if (NdotL > 0.5) {
        celLight = 1.0;     // Highlight zone (Fully lit)
    } else if (NdotL > 0.0) {
        celLight = 0.6;     // Midtone zone
    } else {
        celLight = 0.3;     // Shadow zone (Flat dark ambient)
    }

    // 3. FOAM CREST DETECTION
    // Check if this surface fragment is tilting sharply upward or at a peak wave height.
    // If it is pointing almost straight up relative to a flat plane, or meets our foam criteria, 
    // we paint it solid white to create the "drawn foam" outlines seen in BotW.
    vec3 pureTealWater = fragColour.rgb * celLight;
    vec3 foamWhite = vec3(1.0, 1.0, 1.0);
    
    vec3 finalColour;
    // If the surface normal is sharply angled (cresting wave tip), generate crisp foam outlines
    if (normal.y < 0.75 && normal.y > 0.5) {
        finalColour = foamWhite;
    } else {
        finalColour = pureTealWater;
    }

    // Output with the alpha transparency you built earlier
    outColour = vec4(finalColour, fragColour.a);
}
)"; /*Renderer::Renderer(int w, int h) {
   // Initialise the "video"
   SDL_Init(SDL_INIT_VIDEO);
   // Set version : ) ) ) ) ) ) ) )
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
   // hat does profile mask do?
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
 SDL_GL_CONTEXT_PROFILE_CORE);
   // Create visualisation.
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   window = SDL_CreateWindow("Wave Simulator", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, w, h,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
   glContext = SDL_GL_CreateContext(window);
   glewExperimental = GL_TRUE;
   glewInit();
   glEnable(GL_DEPTH_TEST);
   glViewport(0, 0, w, h);
   shaderProgram = compileShaders();
   setupCubeMesh();
   glGenBuffers(1, &instanceVBO);

   // To this:
 }
 */
// ERROR CHECk
Renderer::Renderer(int w, int h) {
  // 1. Initialize SDL Video and check for failure
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL Init Failed: " << SDL_GetError() << std::endl;
    return;
  }

  // 2. Set modern OpenGL attributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // 3. Create the window frame
  window = SDL_CreateWindow("Wave Simulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  // ===> DIAGNOSTIC CHANGE: Catch the error before it can segfault!
  if (!window) {
    std::cerr << "\n=============================================" << std::endl;
    std::cerr << "CRITICAL ERROR: SDL could not create a window!" << std::endl;
    std::cerr << "Reason: " << SDL_GetError() << std::endl;
    std::cerr << "=============================================\n" << std::endl;
    return;  // Exit safely instead of trying to make a context on a null window
  }

  // 4. Create the context safely
  glContext = SDL_GL_CreateContext(window);
  if (!glContext) {
    std::cerr << "GL Context Creation Failed: " << SDL_GetError() << std::endl;
    return;
  }

  // 5. Fire up GLEW
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "GLEW Init Failed: " << glewGetErrorString(err) << std::endl;
    return;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, w, h);
  shaderProgram = compileShaders();
  setupCubeMesh();
  glGenBuffers(1, &instanceVBO);
  int maxInstances = 50 * 50 * 50;
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(CellInstance), nullptr,
               GL_DYNAMIC_DRAW);
}

Renderer::~Renderer() {
  glDeleteBuffers(1, &instanceVBO);
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteProgram(shaderProgram);

  // 2. Clean up SDL (in reverse order of creation)
  SDL_GL_DeleteContext(
      glContext);  // Assuming you name your context variable this
  SDL_DestroyWindow(window);
  SDL_Quit();
}
GLuint Renderer::compileShaders() {
  // Create identities for shaders.
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint faceShader = glCreateShader(GL_FRAGMENT_SHADER);
  // Assign the shaders using their identity and defintion
  glShaderSource(vertexShader, 1, &vertSrc, NULL);
  glShaderSource(faceShader, 1, &fragSrc, NULL);

  // Compile
  glCompileShader(vertexShader);
  glCompileShader(faceShader);

  // Start the rendering.

  GLuint engine = glCreateProgram();

  glAttachShader(engine, vertexShader);
  glAttachShader(engine, faceShader);

  glLinkProgram(engine);

  // Once all the definitions are made delete the shaders.
  glDeleteShader(vertexShader);
  glDeleteShader(faceShader);
  // Returns the GPU ticket for the engine.
  return engine;
}

void Renderer::setupCubeMesh() {
  // Define the 36 vertices. (3 per triangle 2 triangles per face, and 6 faces
  // yayayaya)
  float vertices[] = {
      // Back face
      -0.485f, -0.485f, -0.485f, 0.485f, -0.485f, -0.485f, 0.485f, 0.485f,
      -0.485f, 0.485f, 0.485f, -0.485f, -0.485f, 0.485f, -0.485f, -0.485f,
      -0.485f, -0.485f,

      // Front face
      -0.485f, -0.485f, 0., 0.485f, -0.485f, 0.485f, 0.485f, 0.485f, 0.485f,
      0.485f, 0.485f, 0.485f, -0.485f, 0.485f, 0.485f, -0.485f, -0.485f, 0.485f,

      // Left face
      -0.485f, 0.485f, 0.485f, -0.485f, 0.485f, -0.485f, -0.485f, -0.485f,
      -0.485f, -0.485f, -0.485f, -0.485f, -0.485f, -0.485f, 0.485f, -0.485f,
      0.485f, 0.485f,

      // Right face
      0.485f, 0.485f, 0.485f, 0.485f, 0.485f, -0.485f, 0.485f, -0.485f, -0.485f,
      0.485f, -0.485f, -0.485f, 0.485f, -0.485f, 0.485f, 0.485f, 0.485f, 0.485f,

      // Bottom face
      -0.485f, -0.485f, -0.485f, 0.485f, -0.485f, -0.485f, 0.485f, -0.485f,
      0.485f, 0.485f, -0.485f, 0.485f, -0.485f, -0.485f, 0.485f, -0.485f,
      -0.485f, -0.485f,

      // Top face
      -0.485f, 0.485f, -0.485f, 0.485f, 0.485f, -0.485f, 0.485f, 0.485f, 0.485f,
      0.485f, 0.485f, 0.485f, -0.485f, 0.485f, 0.485f, -0.485f, 0.485f,
      -0.485f};
  glGenVertexArrays(1, &cubeVAO);
  glBindVertexArray(cubeVAO);

  glGenBuffers(1, &cubeVBO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
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
  glClearColor(0.02f, 0.05f, 0.15f, 1.0f);

  // soft twilight blue-grey
  // glClearColor(0.08f, 0.10f, 0.18f, 1.0f);

  // warm sunset
  // glClearColor(0.15f, 0.08f, 0.05f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shaderProgram);  // first

  // build instances
  std::vector<CellInstance> instances;
  instances.reserve(grid.width * grid.height * grid.length / 4);

  for (int z = 0; z < grid.length; z++)
    for (int y = 3; y < grid.height - 3; y++)
      for (int x = 0; x < grid.width; x++) {
        int i = grid.index(x, y, z);

        if (grid.solid[i]) {
          instances.push_back(
              {(float)x, (float)y, (float)z, 0.4f, 0.4f, 0.4f, 1.0f});
          continue;
        }

        float val = grid.current[i];
        bool isTopLayer = (y == grid.height - 1);

        // edge fade
        float edgeX = std::min(x, grid.width - 1 - x) / 3.0f;
        float edgeZ = std::min(z, grid.length - 1 - z) / 3.0f;
        float edgeFade = std::min(std::min(edgeX, edgeZ), 1.0f);

        if (isTopLayer) {
          // always render surface — base water colour, brighten with waves
          float intensity = std::min(std::abs(val) * 3.5f, 1.0f);
          float g = 0.15f + intensity * 0.70f;
          float b = 0.45f + intensity * 0.50f;
          float a = 0.75f * edgeFade;
          instances.push_back({(float)x, (float)y, (float)z, 0.0f, g, b, a});

        } else {
          // sub-surface — only render if the cell above is active
          if (y >= grid.height - 30 + 10) continue;
          float valAbove = grid.current[grid.index(x, y + 1, z)];
          if (std::abs(valAbove) >= threshold) continue;

          float intensity = std::min(std::abs(val) * 3.5f, 1.0f);
          float g = 0.05f + intensity * 0.60f;
          float b = 0.30f + intensity * 0.55f;
          float a = (0.3f + intensity * 0.4f) * edgeFade;
          instances.push_back({(float)x, (float)y, (float)z, 0.0f, g, b, a});
        }
      }

  if (instances.empty()) return;

  // upload instance data
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(CellInstance),
                  instances.data());

  // bind VAO and set instance attribute pointers
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance),
                        (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance),
                        (void*)12);
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  // uniforms
  glm::mat4 proj = glm::perspective(
      glm::radians(60.0f), (float)800.0f / (float)800.0f, 0.1f, 500.0f);
  glm::mat4 view = glm::lookAt(
      glm::vec3(grid.width / 2, grid.height * 1.5f, grid.length * 2),
      glm::vec3(grid.width / 2, grid.height / 2, grid.length / 2),
      glm::vec3(0, 1, 0));
  glm::mat4 mvp = proj * view;
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"), 1, GL_FALSE,
                     glm::value_ptr(mvp));
  glUniform3f(glGetUniformLocation(shaderProgram, "uLightDir"), 1.0f, 2.0f,
              1.0f);

  glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instances.size());
  glBindVertexArray(0);
}
void Renderer::present() { SDL_GL_SwapWindow(window); }
