#ifndef AIR_H
#define AIR_H

#include "stdint.h"
#include <vector>
#include <cstring>
#include <algorithm>

constexpr unsigned int SCREEN_WIDTH = 840;
constexpr unsigned int SCREEN_HEIGHT = 600;

constexpr unsigned int AIR_CELL_SIZE = 4;
constexpr unsigned int AIR_XRES = SCREEN_WIDTH / AIR_CELL_SIZE;
constexpr unsigned int AIR_YRES = SCREEN_HEIGHT / AIR_CELL_SIZE;

// Simulation constants
// ---------------------------------
constexpr float GRAVITY = -9.81f;
constexpr float dt = 1.0f;
constexpr std::size_t DIVERGENCE_ITERATIONS = 100;
constexpr float DIVERGENCE_OVERRELAXATION = 1.9f;
constexpr float FLUID_DENSITY = 1000.0f;


// TODO: explaination of symbols
// like h1 = 1/h
// h = cell size
// i = grid x
// j = grid y
// why staggered grid
// s = wall


class Air {
public:
    const float density = FLUID_DENSITY;

    float vx[AIR_YRES][AIR_XRES];
    float vy[AIR_YRES][AIR_XRES];
    float new_vx[AIR_YRES][AIR_XRES];
    float new_vy[AIR_YRES][AIR_XRES];

    float pressure[AIR_YRES][AIR_XRES]; // For display only, not needed for simulation (yet)
    bool walls[AIR_YRES][AIR_XRES]; // 1 = wall (obstacle), 0 = empty

    void clear();
    void update(const float dt, const float gravity) {
        apply_forces(dt, gravity);
        memset(pressure, 0.0f, sizeof(pressure));
        solve_incompressibility(DIVERGENCE_ITERATIONS, dt);
        fill_edges();
        advect_velocities(dt);
    }

    Air();
    ~Air();

private:
    void apply_forces(float dt, float gravity);
    void solve_incompressibility(std::size_t iterations, float dt);
    void fill_edges();
    float sample_field_bilinear(float x, float y, float field[AIR_YRES][AIR_XRES], float dx, float dy);
    float avg_vx_at_edge(int x, int y);
    float avg_vy_at_edge(int x, int y);
    void advect_velocities(float dt);
};

#endif