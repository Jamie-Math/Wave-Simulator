#pragma once
#include <vector>

class Grid {
public:
    int width, height, depth;

    // two flat arrays for double buffering
    std::vector<float> current;
    std::vector<float> previous;
    std::vector<bool>  solid;

    Grid(int w, int h, int d);

    // convert 3D coords to flat index
    // formula: z * width * height + y * width + x
    int index(int x, int y, int z) const;

    // ping-pong the two buffers each step
    // just swap the vectors — no data copied
    void swap();

    // mark a box region as solid obstacle
    void addSolidBox(int x0, int y0, int z0, int x1, int y1, int z1);

    // inject a wave impulse at a point
    void addImpulse(int x, int y, int z, float value);
};