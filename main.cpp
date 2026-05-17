#include "Game.h"

int main() {

    constexpr int screenW = 1400;
    constexpr int screenH = 1200;

    InitWindow(screenW, screenH, "Chess");
    Image icon = LoadImage("../images/chess_icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BROWN);

            game.loop();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
