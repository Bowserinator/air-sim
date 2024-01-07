#include "raylib.h"
#include "Air.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame(Air &air);

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bowser's Air Sim");
    Air air;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    while (!WindowShouldClose())
        UpdateDrawFrame(air);
#endif

    CloseWindow();
    return 0;
}


void UpdateDrawFrame(Air &air) {
    air.update();

    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangle(10, 10, 10, 10, WHITE);

    DrawFPS(10, 10);

    EndDrawing();
}
