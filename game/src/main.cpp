#include "raylib.h"
#include "Air.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame(Air &air, int &i);

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bowser's Air Sim");
    Air air;

    auto y=  10;
        // air.cells[y][10].density = 600.9f;
        // air.cells[y][10].vx = 10.0f;
        // air.cells[y][10].vy = 1.0f;
    int i = 0;


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(300);

    while (!WindowShouldClose())
        UpdateDrawFrame(air, i);
#endif

    CloseWindow();
    return 0;
}


void UpdateDrawFrame(Air &air, int &i) {
    
    static float time = 0.0f;

    int R = 10;
    if (time < 0.05) {
        for (int y = AIR_YRES / 2 - R; y < AIR_YRES  / 2 + R; y++) {
            // air.pressure[y][AIR_XRES - 25] += 10.0f;
            air.vx[y][AIR_XRES - 5] = -5.5;
            air.vy[y][AIR_XRES - 5] = 0.00;

            // if (time > 5.0f) {
            //     air.vy[y][AIR_XRES - 5] = 0.0f;
            //     air.vx[y][AIR_XRES - 5] = 0.0f;
            // }
        }
        time += 0.01;
    }


    R = 10;
    for (int y = AIR_YRES / 2 - R; y < AIR_YRES / 2 + R; y++) {
        for (auto x = (int)( AIR_XRES * 0.6 - R); x < AIR_XRES  * 0.6 + R; x++) {
            if ((y - AIR_YRES / 2) * (y - AIR_YRES / 2) + (x - AIR_XRES * 0.6) * (x - AIR_XRES * 0.6) > R * R)
                continue;
            //air.vx[y][x] = 0.0f;
           // air.pressure[y][x] = BASE_DENSITY;
           // air.vy[y][x] = 0.0f;
            air.walls[y][x] = true;
        }

        //air.cells[y][10].vx = 1.0f;
        //air.cells[y][10].vy = 0.0f;
    }

    // for (int y = AIR_YRES / 2; y < AIR_YRES / 4 * 3; y++) {
    //     for (auto x = AIR_XRES / 4 * 3; x < AIR_XRES  * 3/ 4 + 5; x++) {
    //         air.cells[y][x].vx = 0.0f;
    //         air.cells[y][x].density = BASE_DENSITY;
    //         air.cells[y][x].vy = 0.0f;
    //     }

    //     //air.cells[y][10].vx = 1.0f;
    //     //air.cells[y][10].vy = 0.0f;
    // }
    air.update(dt, 0.0f);
    // i++;

    BeginDrawing();
    ClearBackground(BLACK);

    if (false) {
        for (auto y = 0; y < AIR_YRES; y++)
        for (auto x = 0; x < AIR_XRES; x++) {
            constexpr float MAX_P = 0.05f;
            // const float alpha = std::max(-MAX_P, std::min(MAX_P, air.pressure[y][x])) * 255.0f / MAX_P;
            const float alpha = std::max(-MAX_P, std::min(MAX_P, air.vx[y][x])) * 255.0f / MAX_P;
            const float alpha2 = std::max(-MAX_P, std::min(MAX_P, air.vy[y][x])) * 255.0f / MAX_P;

            // if (alpha > 0)
            if (air.walls[y][x])
                DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE, WHITE);
            else
                DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
                    Color{ .r = (unsigned char)std::abs(alpha), .g = (unsigned char)std::abs(alpha2), .b = 0, .a = 255  });
            // else
            //     DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
            //         Color{ .r = (unsigned char)(-alpha2), .g = 0, .b = (unsigned char)-alpha, .a = 255  });
        }
    } else {
        for (auto y = 0; y < AIR_YRES; y++)
        for (auto x = 0; x < AIR_XRES; x++) {
            constexpr float MAX_P = 35.0f;
            const float alpha = std::max(-MAX_P, std::min(MAX_P, air.pressure[y][x])) * 255.0f / MAX_P;
   
            // if (alpha > 0)
            if (air.walls[y][x])
                DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE, WHITE);
            else if (alpha > 0)
                DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
                    Color{ .r = (unsigned char)std::abs(alpha), .g = 0, .b = 0, .a = 255  });
            else
                DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
                    Color{ .r = 0, .g = 0, .b = (unsigned char)-alpha, .a = 255  });
        }
    }

    DrawFPS(10, 10);

    auto pos = GetMousePosition();
    auto py = (int)pos.y / AIR_CELL_SIZE;
    auto px = (int)pos.x / AIR_CELL_SIZE;
    float p = 0.0f;
    float fx = 0.0f;
    float fy = 0.0f;
    
    if (py > 0 && py < AIR_YRES && px > 0 && px < AIR_XRES) {
        p = air.pressure[py][px];
        fx = (air.vx[py][px]);
        fy = (air.vy[py][px]);
        // fx = air.fx(air.cells, px, py).density;
    }
    DrawText(TextFormat(": %f  %f, %f", p, fx, fy), 10, 30, 30, WHITE);
    // DrawText(TextFormat(": %f", fx), 10, 60, 30, WHITE);

    EndDrawing();
}
