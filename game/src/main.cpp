#include "raylib.h"
#include "Air.h"

#include <algorithm>
#include <iostream>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame(Air &air, int &i);

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bowser's Air Sim");
    Air air;

    auto y=  10;
        air.cells[y][10].density = 30.9f;
        air.cells[y][10].vx = 1.0f;
        air.cells[y][10].vy = 1.0f;
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
    for (int y = 10; y <21; y++) {
        // air.cells[y][10].density = 10.0f;
        // air.cells[y][10].vx = 1.0f;
        // air.cells[y][10].vy = 0.0f;
    }
    air.update(i);
    i++;

    BeginDrawing();
    ClearBackground(BLACK);

    for (auto y = 0; y < AIR_YRES; y++)
    for (auto x = 0; x < AIR_XRES; x++) {
        constexpr float MAX_P = 4.0f;
        const float alpha = std::max(-MAX_P, std::min(MAX_P, air.cells[y][x].density - BASE_DENSITY)) * 255.0f / MAX_P;

        if (alpha > 0)
            DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
                Color{ .r = (unsigned char)alpha, .g = 0, .b = 0, .a = 255  });
        else
            DrawRectangle(x * AIR_CELL_SIZE, y * AIR_CELL_SIZE, AIR_CELL_SIZE, AIR_CELL_SIZE,
                Color{ .r = 0, .g = 0, .b = (unsigned char)-alpha, .a = 255  });
    }

    DrawFPS(10, 10);

    auto pos = GetMousePosition();
    auto py = (int)pos.y / AIR_CELL_SIZE;
    auto px = (int)pos.x / AIR_CELL_SIZE;
    float p = 0.0f;
    float fx = 0.0f;
    float fy = 0.0f;
    
    if (py > 0 && py < AIR_YRES && px > 0 && px < AIR_XRES) {
        p = air.cells[py][px].density;
        fx = air.fx(air.cells[py][px]).density;
        fy = air.fy(air.cells[py][px]).vy;
        // fx = air.fx(air.cells, px, py).density;
    }
    DrawText(TextFormat(": %f  %f, %f", p, fx, fy), 10, 30, 30, WHITE);
    // DrawText(TextFormat(": %f", fx), 10, 60, 30, WHITE);

    EndDrawing();
}
