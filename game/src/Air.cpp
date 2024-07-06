#include "Air.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <numeric>

Air::Air() {
    clear();
}

Air::~Air() {}

void Air::clear() {
    memset(walls, 0, sizeof(walls));
    memset(vx, 0.0f, sizeof(vx));
    memset(vy, 0.0f, sizeof(vy));
    memset(new_vx, 0.0f, sizeof(new_vx));
    memset(new_vy, 0.0f, sizeof(new_vy));
    memset(pressure, 0.0f, sizeof(pressure));
}


void Air::apply_forces(float dt, float gravity) {
    for (auto y = 1; y < AIR_YRES - 1; y++)
    for (auto x = 1; x < AIR_XRES - 1; x++) {
        if (!walls[y][x] && !walls[y + 1][x])
            vy[y][x] += gravity * dt;
    }
}


void Air::solve_incompressibility(std::size_t iterations, float dt) {
    float cp = density * AIR_CELL_SIZE / dt; // TODO: what is this for

    for (auto iter = 0; iter < iterations; iter++) {
        for (auto y = 1; y < AIR_YRES - 1; y++)
        for (auto x = 1; x < AIR_XRES - 1; x++) {
            if (walls[y][x]) continue;

            auto no_wall_left   = walls[y][x - 1] ? 0 : 1;
            auto no_wall_right  = walls[y][x + 1] ? 0 : 1;
            auto no_wall_top    = walls[y - 1][x] ? 0 : 1;
            auto no_wall_bottom = walls[y + 1][x] ? 0 : 1;

            // Number of directly adjacent cells that are not wall cells
            auto num_non_walls = no_wall_left + no_wall_right + no_wall_bottom + no_wall_top;

            // This space is fully surrounded by walls, cannot fix divergence
            if (num_non_walls == 0)
                continue;

            float divergence = vx[y][x + 1] - vx[y][x] + vy[y + 1][x] - vy[y][x];
            float p = -divergence / num_non_walls * DIVERGENCE_OVERRELAXATION;
            pressure[y][x] += cp * p;

            vx[y][x]     -= no_wall_left   * p;
            vx[y][x + 1] += no_wall_right  * p;
            vy[y][x]     -= no_wall_top    * p;
            vy[y + 1][x] += no_wall_bottom * p;
        }
    }
}


void Air::fill_edges() {
    // for (auto x = 0; x < AIR_XRES; x++) {
    //     vx[0][x] = vx[1][x];
    //     vx[AIR_YRES - 1][x] = vx[AIR_YRES - 2][x];
    // }
    // for (auto y = 0; y < AIR_YRES; y++) {
    //     vy[y][0] = vy[y][1];
    //     vy[y][AIR_XRES - 1] = vy[y][AIR_XRES - 2];
    // }
}


float Air::sample_field_bilinear(float x, float y, float field[AIR_YRES + 1][AIR_XRES + 1], float dx, float dy) {
    float CELL_SIZE_INV = 1.0f / AIR_CELL_SIZE;
    float HALF_CELL_SIZE = 0.5f * AIR_CELL_SIZE;

    x = std::max(std::min(x, (float)AIR_CELL_SIZE * AIR_XRES), (float)AIR_CELL_SIZE);
    y = std::max(std::min(y, (float)AIR_CELL_SIZE * AIR_YRES), (float)AIR_CELL_SIZE);

    // TODO: explaination

    // x0, y0 is the top left points out of the 4 points we use to perform
    // bilinear interpolation. We first take (x - offset) / CELL_SIZE (clamping to be in screen bounds)
    // (and same for y) to get which grid coordinate it is. Offset depends on whether
    // we are interpolating vx or dy (depending on edge)

    // tx, ty is basically remainder of (x - dx) % CELL_SIZE normalized by CELL_SIZE between 0 - 1
    // similarly, x1, y1 is the bottom right point

    unsigned int x0 = std::min((unsigned int)((x - dx) * CELL_SIZE_INV), AIR_XRES - 1);
    float tx = ((x - dx) - x0 * AIR_CELL_SIZE) * CELL_SIZE_INV;
    unsigned int x1 = std::min(x0 + 1, AIR_XRES - 1);

    unsigned int y0 = std::min((unsigned int)((y - dy) * CELL_SIZE_INV), AIR_YRES - 1);
    float ty = ((y - dy) - y0 * AIR_CELL_SIZE) * CELL_SIZE_INV;
    unsigned int y1 = std::min(y0 + 1, AIR_YRES - 1);

    float sx = 1.0 - tx;
    float sy = 1.0 - ty;

    return sx * sy * field[y0][x0] +
           tx * sy * field[y0][x1] +
           tx * ty * field[y1][x1] +
           sx * ty * field[y1][x0];
}


float Air::avg_vx_at_edge(int x, int y) {
    //TODO: visualization
    return (vx[y - 1][x] + vx[y][x] + vx[y - 1][x + 1] + vx[y][x + 1]) * 0.25f;
}


/// @brief Get vy average at the middle of a vertical edge, the edge is
///        is the left edge of the cell at x,y
/// @param x
/// @param y 
/// @return 
float Air::avg_vy_at_edge(int x, int y) {
    // Visualization:
    //  +--- (Y1) ---+--- (Y2) ---+
    //  |            |            |
    //  |           (A)    [X]    |
    //  |            |            |
    //  +--- (Y3) ---+--- (Y4) ---+

    // Here [X] represents the cell with center [y][x].
    //   Y1 = vy[Y][X - 1]
    //   Y2 = vy[Y][X]
    //   Y3 = vy[Y + 1][X - 1]
    //   Y4 = vy[Y + 1][X]
    // We want to find vy at (A), which is just the average of Y1,Y2,Y3,Y4

    return (vy[y][x - 1] + vy[y][x] + vy[y + 1][x - 1] + vy[y + 1][x]) * 0.25f;
}


/// @brief Advect velocities
/// TODO: explaination of backwards langargnaisn or something
/// @param dt Delta time
void Air::advect_velocities(float dt) {
    float HALF_CELLSIZE = AIR_CELL_SIZE * 0.5f; // h2

    memset(new_vx, 0.0f, sizeof(vx));
    memset(new_vy, 0.0f, sizeof(vy));

    // memcpy(new_vx, vx, sizeof(vx));
    // memcpy(new_vy, vy, sizeof(vy));

    for (auto y = 1; y < AIR_YRES; y++)
    for (auto x = 1; x < AIR_XRES; x++) {
        // Project vx, vy separately since we start on different edges
        // (vx are on the vertical edges, but vy on the horizontal)
        // Note: vx[y][x] is left edge of the cell, vy[y][x] is top edge of the cell
        //       (the cell being the cell at [y][x])

        // vx:
        if (!walls[y][x] && !walls[y][x - 1] && y < AIR_YRES - 1) {
            // Not grid coordinates, actual physical space coordinates
            // Since we are in the middle of the left edge (of a cell)
            // our y coordinate is actually halfway down (hence the + half cell size)
            float real_x = x * AIR_CELL_SIZE;
            float real_y = y * AIR_CELL_SIZE + HALF_CELLSIZE;
            float vx_here = vx[y][x];
            float vy_here = avg_vy_at_edge(x, y);

            real_x -= dt * vx_here;
            real_y -= dt * vy_here;

            float k1_vx = sample_field_bilinear(real_x, real_y, vx, 0.0f, HALF_CELLSIZE);
            float k1_vy = sample_field_bilinear(real_x, real_y, vy, HALF_CELLSIZE, 0.0f);

            float k2_vx = sample_field_bilinear(real_x + dt / 2 * k1_vx, real_y + dt / 2 * k1_vy,
                vx, 0.0f, HALF_CELLSIZE);
            float k2_vy = sample_field_bilinear(real_x + dt / 2 * k1_vx, real_y + dt / 2 * k1_vy,
                vy, HALF_CELLSIZE, 0.0f);

            float k3_vx = sample_field_bilinear(real_x + dt / 2 * k2_vx, real_y + dt / 2 * k2_vy,
                vx, 0.0f, HALF_CELLSIZE);
            float k3_vy = sample_field_bilinear(real_x + dt / 2 * k2_vx, real_y + dt / 2 * k2_vy,
                vy, HALF_CELLSIZE, 0.0f);

            float k4_vx = sample_field_bilinear(real_x + dt * k3_vx, real_y + dt * k3_vy,
                vx, 0.0f, HALF_CELLSIZE);


            new_vx[y][x] = 1/6.0f * (k1_vx + 2 * k2_vx + 2 * k3_vx + k4_vx);
            
            // new_vx[y][x] = sample_field_bilinear(real_x, real_y, vx, 0.0f, HALF_CELLSIZE);
        }

        // vy:
        if (!walls[y][x] && !walls[y - 1][x] && x < AIR_XRES - 1) {
            // Not grid coordinates, actual physical space coordinates
            // Since we are in the middle of the top edge (of a cell)
            // we offset the x coordinate by half a cell
            float real_x = x * AIR_CELL_SIZE + HALF_CELLSIZE;
            float real_y = y * AIR_CELL_SIZE;
            float vx_here = avg_vx_at_edge(x, y);
            float vy_here = vy[y][x];

            real_x -= dt * vx_here;
            real_y -= dt * vy_here;

            float k1_vx = sample_field_bilinear(real_x, real_y, vx, 0.0f, HALF_CELLSIZE);
            float k1_vy = sample_field_bilinear(real_x, real_y, vy, HALF_CELLSIZE, 0.0f);

            float k2_vx = sample_field_bilinear(real_x + dt / 2 * k1_vx, real_y + dt / 2 * k1_vy,
                vx, 0.0f, HALF_CELLSIZE);
            float k2_vy = sample_field_bilinear(real_x + dt / 2 * k1_vx, real_y + dt / 2 * k1_vy,
                vy, HALF_CELLSIZE, 0.0f);

            float k3_vx = sample_field_bilinear(real_x + dt / 2 * k2_vx, real_y + dt / 2 * k2_vy,
                vx, 0.0f, HALF_CELLSIZE);
            float k3_vy = sample_field_bilinear(real_x + dt / 2 * k2_vx, real_y + dt / 2 * k2_vy,
                vy, HALF_CELLSIZE, 0.0f);

            float k4_vy = sample_field_bilinear(real_x + dt * k3_vx, real_y + dt * k3_vy,
                vy, HALF_CELLSIZE, 0.0f);

            new_vy[y][x] = 1/6.0f * (k1_vy + 2 * k2_vy + 2 * k3_vy + k4_vy);
            



            // new_vy[y][x] = sample_field_bilinear(real_x, real_y, vy, HALF_CELLSIZE, 0.0f);
        }
    }

    // new_vx new_vy are a scratch buffer to store new values in
    // once we are done we set vx = new_vx, vy = new_vy
    // To avoid extra copying, we can just swap them
    memcpy(vx, new_vx, sizeof(vx));
    memcpy(vy, new_vy, sizeof(vy)); // TODO
    // std::swap(new_vx, vx);
    // std::swap(new_vy, vy);
}
