#ifndef AIR_H
#define AIR_H

#include "stdint.h"
#include <vector>

constexpr unsigned int SCREEN_WIDTH = 1440;
constexpr unsigned int SCREEN_HEIGHT = 800;

constexpr unsigned int AIR_CELL_SIZE = 3;
constexpr unsigned int AIR_XRES = SCREEN_WIDTH / AIR_CELL_SIZE;
constexpr unsigned int AIR_YRES = SCREEN_HEIGHT / AIR_CELL_SIZE;

constexpr uint8_t PRESSURE_IDX = 0;
constexpr uint8_t VX_IDX = 1;
constexpr uint8_t VY_IDX = 2;

constexpr float BASE_DENSITY = 10.5f;

using coord_t = uint16_t;

struct AirCell {
    float density;
    float vx; // x, y momentum
    float vy;

    friend inline AirCell operator*(float x, const AirCell &c) {
        return AirCell { c.density * x, c.vx * x, c.vy * x };
    }
    friend inline AirCell operator*(const AirCell &c, float x) {
        return AirCell { c.density * x, c.vx * x, c.vy * x };
    }
    friend inline AirCell operator-(const AirCell &lhs, const AirCell &rhs) {
        return AirCell { lhs.density - rhs.density, lhs.vx - rhs.vx, lhs.vy - rhs.vy };
    }
    friend inline AirCell operator+(const AirCell &lhs, const AirCell &rhs) {
        return AirCell { lhs.density + rhs.density, lhs.vx + rhs.vx, lhs.vy + rhs.vy };
    }
};

using grid = std::vector<std::vector<AirCell>>;

class Air {
public:
    //AirCell cells[AIR_YRES][AIR_XRES];
    //AirCell out_cells[AIR_YRES][AIR_XRES];
    //AirCell out_cells2[AIR_YRES][AIR_XRES];
    grid cells;
    grid out_cells;
    grid out_cells2;

    void clear();
    void update(int i);

    AirCell fx(AirCell &cell);
    AirCell fy(AirCell &cell);

    AirCell getFluxX(AirCell &left, AirCell &right);
    AirCell getFluxY(AirCell &left, AirCell &right);

    Air();
    ~Air();

private:
    void setEdgesAndWalls();
    // void projectFutureValueX(const coord_t x, const coord_t y, const uint8_t data_idx);
    // void projectFutureValueY(const coord_t x, const coord_t y, const uint8_t data_idx);
    void projectFutureValue(const coord_t x, const coord_t y);
};

#endif