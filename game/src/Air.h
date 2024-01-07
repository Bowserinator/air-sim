#ifndef AIR_H
#define AIR_H

constexpr unsigned int SCREEN_WIDTH = 1440;
constexpr unsigned int SCREEN_HEIGHT = 800;

constexpr unsigned int AIR_CELL_SIZE = 4;
constexpr unsigned int AIR_XRES = SCREEN_WIDTH / AIR_CELL_SIZE;
constexpr unsigned int AIR_YRES = SCREEN_HEIGHT / AIR_CELL_SIZE;

constexpr unsigned int PRESSURE_IDX = 0;
constexpr unsigned int VX_IDX = 1;
constexpr unsigned int VY_IDX = 2;

struct AirCell {
    float data[3];
};

class Air {
public:
    AirCell cells[AIR_YRES][AIR_XRES];
    AirCell out_cells[AIR_YRES][AIR_XRES];

    void clear();
    void update();

    Air();

private:
    void setEdgesAndWalls();
    void setPressureFromVelocity();
    void setVelocityFromPressure();
    void diffusion();
    void advection();
};

#endif