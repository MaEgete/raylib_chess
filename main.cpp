#include <iostream>
#include "raylib.h"
#include <vector>
#include <utility>

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
        W_KING = 1,   // W König = 1
        W_QUEEN,      // W Königin = 2
        W_ROOK,       // W Turm = 3
        W_BISHOP,     // W Läufer = 4
        W_KNIGHT,     // W Pferd = 5
        W_PAWN,       // W Bauer = 6

        B_KING,       // B König = 7 (1 + 6)
        B_QUEEN,      // B Königin = 8
        B_ROOK,       // B Turm = 9
        B_BISHOP,     // B Läufer = 10
        B_KNIGHT,     // B Pferd = 11
        B_PAWN,       // B Bauer = 12
    };

    Texture2D pieceSpritesheet;

    // Fuer die Bilder der Figuren
    enum class PieceSprite {
        W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_KING, W_QUEEN,
        B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_KING, B_QUEEN,
        COUNT
    };

    std::vector<Rectangle> pieceSprites{static_cast<int>(PieceSprite::COUNT)};

    int gMap[8][8] = {
        {9,11,10,8,7,10,11,9},
        {12,12,12,12,12,12,12,12},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {6,6,6,6,6,6,6,6},
        {3,5,4,2,1,4,5,3},
    };


    // Angeklicktes Feld markieren
    std::pair<int, int> clickedField = {-1, -1};

    // Alle moeglichen Zuege markieren
    std::vector<std::pair<int, int>> possibleMoves;



public:

    Game(int fieldlength = 600) : fieldlength{fieldlength}, blocksize{fieldlength / 8} {
        this->midX = GetScreenWidth() / 2;
        this->midY = GetScreenHeight() / 2;

        fieldX = midX - (fieldlength / 2);
        fieldY = midY - (fieldlength / 2);

        field = Rectangle{static_cast<float>(fieldX), static_cast<float>(fieldY), static_cast<float>(fieldlength), static_cast<float>(fieldlength)};

        pieceSpritesheet = LoadTexture("../images/chesspieces.png");

        // Ganzes Sheet in 12 Bloecke aufteilen
        float tileW = pieceSpritesheet.width / 6.0;
        float tileH = pieceSpritesheet.height / 2.0;
        for (int i = 0; i < 6; i++) {
            pieceSprites[i] = Rectangle{i*tileW, 0, tileW, tileH};
        }
        for (int i = 0; i < 6; i++) {
            pieceSprites[6 + i] = Rectangle{i*tileW, tileH, tileW, tileH};
        }


    }


    ~Game() {
        UnloadTexture(pieceSpritesheet);
    }


    // Spiel zeichnen
    void draw() {
        drawField();
        drawClickedField();
        drawFigures();
    }

    // Regeln
    void update() {
        moveFigure();
    }

    // Spielfeld zeichnen
    void drawField() {
        // Hintergrund Spielfeld
        DrawRectangleRec(field, {206,130,64,255});

        static int offset = 1;

        // Kacheln zeichnen im richtigen Muster
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {

                int nx = fieldX + (blocksize * x);
                int ny = fieldY + (blocksize * y);

                if ((x+offset) % 2 == 0) {
                    DrawRectangle(nx, ny, blocksize, blocksize, {119,119,119,255});
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

    void drawClickedField() {
        if (clickedField.first != -1 && clickedField.second != -1) {
            float nx = this->fieldX + (this->blocksize * clickedField.first);
            float ny = this->fieldY + (this->blocksize * clickedField.second);
            DrawRectangle(nx, ny, this->blocksize, this->blocksize, {255,0,0,255});

            drawPossibleMoves();

        }
    }

    void drawFigures() {

        float scale = 2.5f;

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int nx = fieldX + (blocksize * x) + (blocksize / 6);
                int ny = fieldY + (blocksize * y);

                // Bild von Figuren zeichnen

                switch (static_cast<Players>(gMap[y][x])) {
                    case Players::EMPTY:
                        // Feld ist leer
                        break;

                    case Players::W_KING: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_KING)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::W_QUEEN: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_QUEEN)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::W_ROOK: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_ROOK)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::W_BISHOP: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_BISHOP)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::W_KNIGHT: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_KNIGHT)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::W_PAWN: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::W_PAWN)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }

                    case Players::B_KING: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_KING)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::B_QUEEN: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_QUEEN)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::B_ROOK: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_ROOK)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::B_BISHOP: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_BISHOP)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::B_KNIGHT: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_KNIGHT)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    case Players::B_PAWN: {
                        Rectangle src = pieceSprites[static_cast<int>(PieceSprite::B_PAWN)];
                        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
                        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        break;
                    }
                    default:
                        std::cout << "False field value" << std::endl;
                        break;
                }

            }
        }
    }

    // Spielfeld umdrehen
    void turnaround() {
        // Irgendwas einfallen lassen, sodass nur die View um 180Grad gedreht wird, damit alle Berechnungen noch gleich sind
        // Vllt. mit einer Rotationsmatrix arbeiten
    }

    void moveFigure() {
        // Figur anklicken, dann sollen alle moeglichen Felder in ROT angezeigt werden
        // Auf die Felder dann klicken zum verschieben

        // Erreichbare Felder haben entweder den Zustand Players::EMPTY oder Players::wasAnderes
        // Bei Players::EMPTY soll eine andere Farbe angezeigt werden als bei Players::wasAnderes

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            // Maus hat aufs Spielfeld gedrueckt
            if (mousePos.x > fieldX && mousePos.x < (fieldX + fieldlength)
                && mousePos.y > fieldY && mousePos.y < (fieldY + fieldlength)) {
                std::cout << "Mouse clicked on field" << std::endl;

                // Identifizieren des geklickten Blocks
                float nx = mousePos.x - fieldX;
                float ny = mousePos.y - fieldY;

                int x = static_cast<int>(nx / blocksize);
                int y = static_cast<int>(ny / blocksize);

                std::cout << "x: " << x << " y: " << y << std::endl;

                // Feld speichern
                this->clickedField = {x, y};

                calculatePossibleMoves();

            }
            else {
                // Markiertes Feld wegmachen
                this->clickedField = {-1, -1};
            }
        }


    }

    void calculatePossibleMoves() {
        // Spieler
        auto player = static_cast<Players>(gMap[this->clickedField.second][this->clickedField.first]);

        // Aufgrund des ermittelten Spielers, muessen die jeweiligen Move-Regeln herangezogen werden




        this->possibleMoves.clear();

        // Output = Anzeige aller moeglichen Felder
    }

    void drawPossibleMoves() {

    }


};




int main() {

    const int screenW = 1200;
    const int screenH = 1000;

    InitWindow(screenW, screenH, "chess");

    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BROWN);

        game.update();
        game.draw();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}