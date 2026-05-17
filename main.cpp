#include "Game.h"

int main() {


    constexpr int screenH = 800;
    constexpr int screenW = screenH + screenH / 6;

    InitWindow(screenW, screenH, "Chess");
    Image icon = LoadImage("../images/chess_icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    SetTargetFPS(60);

    Game game(screenH / 2);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BROWN);

            game.loop();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
