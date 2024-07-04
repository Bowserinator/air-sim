#include "Air.h"

#include <cstring>
#include <iostream>
#include <algorithm>

constexpr float dt = 1 / 120.0f; // Delta t timestep
constexpr float dx = static_cast<float>(AIR_CELL_SIZE);
constexpr float C = 1.0f; // Wave speed
constexpr float MAX_V = 10.0f;


Air::Air() {
    clear();
}

Air::~Air() {}

void Air::clear() {
    memset(walls, 0, sizeof(walls));
    memset(vx, 0.0, sizeof(vx));
    memset(vy, 0.0, sizeof(vy));
    memset(nvx, 0.0, sizeof(nvx));
    memset(nvy, 0.0, sizeof(nvy));
    memset(pressure, 0.0, sizeof(pressure));
}

void Air::update(int i) {
    float cp = (float)AIR_CELL_SIZE * dt; // density * grid_size

    for (int n = 0; n < 12; n++) {
        int yStart = true ? 1 : AIR_YRES - 1;
        int yEnd = true ? AIR_YRES - 1 : 1;
        int yInc = true ? 1 : -1;

        int xStart = true ? 1 : AIR_XRES - 1;
        int xEnd = true ? AIR_XRES - 1 : 1;
        int xInc = true ? 1 : -1;

        for (auto y = yStart; y < yEnd; y += yInc)
        for (auto x = xStart; x < xEnd; x += xInc) {
            if (walls[y][x]) continue;

            float divergence = vx[y][x + 1] - vx[y][x] + vy[y + 1][x] - vy[y][x];

            int sx0 = 1 - walls[y][x - 1]; // [(i-1)*n + j];
            int sx1 = 1 - walls[y][x + 1]; // [(i+1)*n + j];
            int sy0 = 1 - walls[y - 1][x]; // [i*n + j-1];
            int sy1 = 1 - walls[y + 1][x]; // [i*n + j+1];
            int s = sx0 + sx1 + sy0 + sy1;

            if (s == 0 ) {
                // vx[y][x] = vx[y][x + 1] = vy[y][x] = vy[y + 1][x] = 0.0;
                continue;
            }

            divergence *= 1.9;
            divergence /= s;

            vx[y][x]     += divergence * cp;
            vx[y][x + 1] -= divergence * cp;
            vy[y][x]     += divergence * cp;
            vy[y + 1][x] -= divergence * cp;

            if (!sx0) vx[y][x] = 0.0f;
            if (!sx1) vx[y][x + 1] = 0.0f;
            if (!sy0) vy[y][x] = 0.0f;
            if (!sy1) vy[y + 1][x] = 0.0f;

            pressure[y][x] = divergence * cp; // 1 = density * grid size
        }
    }

    // Advect velocity
    for (auto y = 1; y < AIR_YRES; y++)
    for (auto x = 1; x < AIR_XRES; x++) {
        if (walls[y][x]) continue;

        float velx = vx[y][x];
        float vely = (vy[y + 1][x] + vy[y][x] + vy[y][x + 1] + vy[y + 1][x + 1]) / 4;
        float newX = (float)x - dt * velx;
        float newY = (float)y - dt * vely;
        int intX = (int)newX;
        int intY = (int)newY;
        newX -= intX;

        // x offset is aligned to center of cell
        newY -= intY;
        newX += 0.5;
        if (newX > 1) newX -= 1;

        if (intX < 1 || intX >= AIR_XRES || intY < 1 || intY >= AIR_YRES)
            continue;

        float w00 = 1 - newX;
        float w10 = 1 - newY;
        float w01 = newX;
        float w11 = newY;
        float interpVelX = w00 * w10 * vx[intY][intX] + w01 * w10 * vx[intY][intX + 1] + w10 * w11 * vx[intY + 1][intX]
            + w11 * w11 * vx[intY+1][intX+1];
        float interpVelY = w00 * w10 * vy[intY][intX] + w01 * w10 * vy[intY][intX + 1] + w10 * w11 * vy[intY + 1][intX]
            + w11 * w11 * vy[intY+1][intX+1];
        nvx[y][x] = interpVelX;
        nvy[y][x] = interpVelY;
    }

    for (auto y = 0; y <= AIR_YRES; y++)
    for (auto x = 0; x <= AIR_XRES; x++) {
        vx[y][x] = nvx[y][x];
        vy[y][x] = nvy[y][x];

        // if (y == 0 || y == AIR_YRES)
        //     vy[y][x] = 0.0;
        // if (x == 0 || x == AIR_XRES)
        //     vx[y][x] = 0.0;
    }
}

