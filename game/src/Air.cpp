#include "Air.h"

#include <cstring>
#include <iostream>
#include <algorithm>

constexpr float dt = 1.0f; // Delta t timestep
constexpr float dx = static_cast<float>(AIR_CELL_SIZE);
constexpr float C = 1.0f; // Wave speed
constexpr float MAX_V = 0.9f;


Air::Air() {
    cells = std::vector<std::vector<AirCell>>(AIR_YRES, std::vector<AirCell>(AIR_XRES));
    out_cells = std::vector<std::vector<AirCell>>(AIR_YRES, std::vector<AirCell>(AIR_XRES));
    out_cells2 = std::vector<std::vector<AirCell>>(AIR_YRES, std::vector<AirCell>(AIR_XRES));

    clear();
}

Air::~Air() {

}

void Air::clear() {
    //memset(cells, 0.0f, sizeof(cells));
   // memset(out_cells, 0.0f, sizeof(cells));
    //memset(out_cells2, 0.0f, sizeof(cells));

    for (auto y = 0; y < AIR_YRES; y++)
    for (auto x = 0; x < AIR_XRES; x++) {
        cells[y][x].density = BASE_DENSITY;
    }
}

void Air::update(int i) {
    for (auto y = 1; y < AIR_YRES - 1; y++)
    for (auto x = 1; x < AIR_XRES - 1; x++) {
        if (i & 1) {
            auto lFlux = getFluxX(cells[y][x + 1], cells[y][x]);
            auto tFlux = getFluxY(cells[y + 1][x], cells[y][x]);
            out_cells[y][x] = cells[y][x] - dt / dx * (tFlux + lFlux); // 
        } else {
            auto lFlux = getFluxX(cells[y][x], cells[y][x - 1]);
            auto tFlux = getFluxY(cells[y][x], cells[y - 1][x]);
            out_cells[y][x] = cells[y][x] - dt / dx * (tFlux + lFlux);
        }
    }

    for (auto y = 1; y < AIR_YRES - 1; y++)
    for (auto x = 1; x < AIR_XRES - 1; x++) {
        AirCell step2;
         if (i & 1) {
            step2 = 0.5f * (cells[y][x] + out_cells[y][x]) - dt / dx * 0.5f * C * (
                getFluxX(out_cells[y][x], out_cells[y][x - 1]) +
                getFluxY(out_cells[y][x], out_cells[y - 1][x]));  // + 
            // step2 = out_cells[y][x];
         } else {
            step2 = 0.5f * (cells[y][x] + out_cells[y][x]) - dt / dx * 0.5f * C * (
                  getFluxX(out_cells[y][x + 1], out_cells[y][x])
                + getFluxY(out_cells[y + 1][x], out_cells[y][x])
            );
         }
        out_cells2[y][x] = step2;
        //out_cells2[y][x].vy = 0.0f;

        out_cells2[y][x].vx = std::clamp(out_cells2[y][x].vx, -MAX_V, MAX_V);
        out_cells2[y][x].vy = std::clamp(out_cells2[y][x].vy, -MAX_V, MAX_V);
        out_cells2[y][x].density = std::clamp(out_cells2[y][x].density, BASE_DENSITY - 0.5f, 100.0f);

        // out_cells2[y][x] = 0.5f * (out_cells[y][x] + cells[y][x]) - dt/dx * 0.5f * (fx(out_cells[y][x]) - fx(out_cells[y][x - 1]));
    }

    for (auto y = 0; y < AIR_YRES - 0; y++)
    for (auto x = 0; x < AIR_XRES - 0; x++) {
        cells[y][x] = out_cells2[y][x];

        // if (cells[y][x].density < BASE_DENSITY)
        //     cells[y][x].density = BASE_DENSITY;

        if (x <= 2 || x >= AIR_XRES - 3) {
            cells[y][x].vx = 0.0;
            cells[y][x].vy = 0.0;
            cells[y][x].density = BASE_DENSITY;
        }
        else if (y <= 2 || y >= AIR_YRES - 3) {
            cells[y][x].vx = 0.0;
            cells[y][x].vy = 0.0;
            cells[y][x].density = BASE_DENSITY;
        }
    }

    // std::swap(cells, out_cells);
}


// Actual sim
// -------------------------------------------

// Horizontal flux
AirCell Air::fx(AirCell &cell) {
    auto density = 0.01f + cell.density; //  std::max(1.0f, cell.density);
    auto vx = cell.vx / density;
    auto vy = cell.vy / density;
    auto pressure = density;

    return AirCell{
        .density = vx * density,
        .vx = vx * vx * density + pressure,
        .vy = vx * vy * density
    };
}

AirCell Air::fy(AirCell &cell) {
    auto density = 0.01f + cell.density; // std::max(1.0f, cell.density);
    auto vx = cell.vx / density;
    auto vy = cell.vy / density;
    auto pressure = density;
    // return AirCell { 0.0, 0.0, 0.0 };

    return AirCell{
        .density = vy * density,
        .vx = vx * vy * density,
        .vy = vy * vy * density + pressure,
    };
}

AirCell Air::getFluxX(AirCell &left, AirCell &right) {
    auto left_flux = fx(left);
    auto right_flux = fx(right);

    auto estimate = (left_flux - right_flux);
    return estimate;
}

AirCell Air::getFluxY(AirCell &left, AirCell &right) {
    auto left_flux = fy(left);
    auto right_flux = fy(right);

    auto estimate = (left_flux - right_flux);

    return estimate;
}

void Air::projectFutureValue(const coord_t x, const coord_t y) {
    // out_cells2[y][x] = 0.5f * (cells[y][x] + out_cells[y][x]) - C * 0.5f * dt / dx * (fx(out_cells, x, y) - fx(out_cells, x - 1, y));
}

