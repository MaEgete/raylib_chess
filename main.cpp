#include <iostream>

#include "raylib.h"

class Game {

private:

    int fieldlength;

    int fieldX;
    int fieldY;

    int blocksize;
    int midX;
    int midY;

    Rectangle field;

    enum class Players : int{
        EMPTY = 0,  // Leeres Feld = 0
        KING = 1,   // König = 1
        QUEEN,      // Königin = 2
        ROOK,       // Turm = 3
        BISHOP,     // Läufer = 4
        KNIGHT,     // Pferd = 5
        PAWN,       // Bauer = 6
    };

    struct FigureImage {
        Image king;
        Image queen;
        Image rook;
        Image bishop;
        Image knight;
        Image pawn;
    };

    int gMap[8][8] = {
        {3,5,4,2,1,4,5,3},
        {6,6,6,6,6,6,6,6},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {6,6,6,6,6,6,6,6},
        {3,5,4,2,1,4,5,3},
    };

public:

    Game(int fieldlength = 400) : fieldlength{fieldlength}, blocksize{fieldlength / 8} {
        this->midX = GetScreenWidth() / 2;
        this->midY = GetScreenWidth() / 2;

        fieldX = midX - (fieldlength / 2);
        fieldY = midY - (fieldlength / 2);

        field = Rectangle{static_cast<float>(fieldX), static_cast<float>(fieldY), static_cast<float>(fieldlength), static_cast<float>(fieldlength)};

        // Bilder in den Struct laden

    }


    ~Game() {
        // Bilder aus dem Speicher laden
    }


    // Spiel zeichnen
    void draw() {
        drawField();
        drawFigures();
    }

    // Regeln
    void update() {

    }

    // Spielfeld zeichnen
    void drawField() {
        // Hintergrund Spielfeld
        DrawRectangleRec(field, WHITE);

        static int offset = 1;

        // Kacheln zeichnen
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {

                int nx = fieldX + (blocksize * x);
                int ny = fieldY + (blocksize * y);

                if ((x+offset) % 2 == 0) {
                    DrawRectangle(nx, ny, blocksize, blocksize, BLACK);
                }
            }
            if (offset == 0) {
                offset = 1;
            }
            else if (offset == 1) {
                offset = 0;
            }
        }

    }

    void drawFigures() {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int nx = fieldX + (blocksize * x);
                int ny = fieldY + (blocksize * y);

                // Bild von Figuren zeichnen

                switch (gMap[y][x]) {
                    case 0:
                        // Feld ist leer
                        break;
                    case 1:
                        // Koenig im Feld zeichnen
                        break;
                    case 2:
                        break;
                    case 3:
                        break;
                    case 4:
                        break;
                    case 5:
                        break;
                    case 6:
                        break;
                }

            }
        }
    }

    // Spielfeld umdrehen
    void turnaround() {

    }


};




int main() {

    const int screenW = 1000;
    const int screenH = 1000;

    InitWindow(screenW, screenH, "chess");

    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(GREEN);

        game.draw();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}