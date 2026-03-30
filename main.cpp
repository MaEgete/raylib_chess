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
        {9 ,11,10,8 ,7 ,10,0,9},
        {12 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,11 ,0 ,0},
        {0 ,12 ,0 ,5 ,0 ,0 ,12 ,12},
        {6 ,0 ,0 ,0 ,0 ,12 ,0 ,6},
        {0 ,0 ,6 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {3 ,5 ,4 ,2 ,1 ,4 ,5 ,3},
    };


    // Angeklicktes Feld markieren
    std::pair<int, int> clickedField = {-1, -1};

    // Alle moeglichen Zuege markieren
    std::vector<std::pair<int, int>> possibleMoves;

    // true == weiss ist am Zug
    // false == schwarz ist am Zug
    bool turn = true;


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
        drawText();
    }

    // Regeln
    void update() {
        moveFigure();
    }

    void drawText() const {
        DrawText(TextFormat("%s to move", (this->turn ? "White" : "Black"), 20), 10, 10, 20, WHITE);
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
            DrawRectangleLines(nx, ny, this->blocksize, this->blocksize, BLACK);

            drawPossibleMoves();

        }
    }

    void drawFiguresHelpMethod(PieceSprite pieceSprite, int nx, int ny, float scale) const {
        Rectangle src = pieceSprites[static_cast<int>(pieceSprite)];
        auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
        DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
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
                        drawFiguresHelpMethod(PieceSprite::W_KING, nx, ny, scale);
                        break;
                    }
                    case Players::W_QUEEN: {
                        drawFiguresHelpMethod(PieceSprite::W_QUEEN, nx, ny, scale);
                        break;
                    }
                    case Players::W_ROOK: {
                        drawFiguresHelpMethod(PieceSprite::W_ROOK, nx, ny, scale);
                        break;
                    }
                    case Players::W_BISHOP: {
                        drawFiguresHelpMethod(PieceSprite::W_BISHOP, nx, ny, scale);
                        break;
                    }
                    case Players::W_KNIGHT: {
                        drawFiguresHelpMethod(PieceSprite::W_KNIGHT, nx, ny, scale);
                        break;
                    }
                    case Players::W_PAWN: {
                        drawFiguresHelpMethod(PieceSprite::W_PAWN, nx, ny, scale);
                        break;
                    }

                    case Players::B_KING: {
                        drawFiguresHelpMethod(PieceSprite::B_KING, nx, ny, scale);
                        break;
                    }
                    case Players::B_QUEEN: {
                        drawFiguresHelpMethod(PieceSprite::B_QUEEN, nx, ny, scale);
                        break;
                    }
                    case Players::B_ROOK: {
                        drawFiguresHelpMethod(PieceSprite::B_ROOK, nx, ny, scale);
                        break;
                    }
                    case Players::B_BISHOP: {
                        drawFiguresHelpMethod(PieceSprite::B_BISHOP, nx, ny, scale);
                        break;
                    }
                    case Players::B_KNIGHT: {
                        drawFiguresHelpMethod(PieceSprite::B_KNIGHT, nx, ny, scale);
                        break;
                    }
                    case Players::B_PAWN: {
                        drawFiguresHelpMethod(PieceSprite::B_PAWN, nx, ny, scale);
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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            // Maus hat aufs Spielfeld gedrueckt
            if (mousePos.x > fieldX && mousePos.x < (fieldX + fieldlength)
                && mousePos.y > fieldY && mousePos.y < (fieldY + fieldlength)) {
                std::cout << "Mouse clicked on field" << std::endl;

                // Identifizieren des geklickten Blocks
                float nx = mousePos.x - static_cast<float>(fieldX);
                float ny = mousePos.y - static_cast<float>(fieldY);

                int x = static_cast<int>(nx / blocksize);
                int y = static_cast<int>(ny / blocksize);

                std::cout << "x: " << x << " y: " << y << std::endl;


                if (this->clickedField.first == x && this->clickedField.second == y) {
                    this->clickedField = {-1, -1};
                }
                else {
                    // Feld speichern
                    this->clickedField = {x, y};
                }


                calculatePossibleMoves();

            }
            else {
                // Markiertes Feld wegmachen
                this->clickedField = {-1, -1};
            }
        }


    }

    void calculatePossibleMoves() {
        // Spieler auf dem geklickten Feld

        int x = this->clickedField.first;
        int y = this->clickedField.second;


        if (x == -1 && y == -1) {
            std::cout << "No field clicked" << std::endl;
            return;
        }

        this->possibleMoves.clear();

        auto player = static_cast<Players>(gMap[y][x]);

        // Aufgrund des ermittelten Spielers, muessen die jeweiligen Move-Regeln herangezogen werden

        switch (player) {

            case Players::W_KING: {
                break;
            }
            case Players::B_KING: {
                break;
            }
            case Players::W_QUEEN: {
                break;
            }
            case Players::B_QUEEN: {
                break;
            }

            case Players::W_ROOK: {
                std::cout << "W Rook" << std::endl;

                // While-Schleife um zu ueberpruefen, wie viele Felder frei sind
                // Richtung nach oben
                int nx = x, ny = y;
                while (ny > 0){
                    nx = x;
                    ny -= 1;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) || ny < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;
                while (ny < 7){
                    nx = x;
                    ny += 1;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) || ny > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;
                while (nx > 0){
                    nx -= 1;
                    ny = y;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) || nx < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;
                while (nx > 0){
                    nx += 1;
                    ny = y;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) || nx > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) {
                        break;
                    }
                }

                break;
            }
            case Players::B_ROOK: {
                std::cout << "B Rook" << std::endl;

                // While-Schleife um zu ueberpruefen, wie viele Felder frei sind
                // Richtung nach oben
                int nx = x, ny = y;
                while (ny > 0){
                    nx = x;
                    ny -= 1;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) || ny < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;
                while (ny < 7){
                    nx = x;
                    ny += 1;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) || ny > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;
                while (nx > 0){
                    nx -= 1;
                    ny = y;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) || nx < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) {
                        break;
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;
                while (nx > 0){
                    nx += 1;
                    ny = y;
                    if ((static_cast<Players>(gMap[ny][nx]) >= Players::B_KING) || nx > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (static_cast<Players>(gMap[ny][nx]) >= Players::W_KING && static_cast<Players>(gMap[ny][nx]) < Players::B_KING) {
                        break;
                    }
                }

                break;
            }

            case Players::W_BISHOP: {
                break;
            }
            case Players::B_BISHOP: {
                break;
            }

            case Players::W_KNIGHT: {

                int nx = x;
                int ny = y;


                if ((nx+1) < 8 && (ny-2) >= 0 && (static_cast<Players>(gMap[ny-2][nx+1]) >= Players::B_KING || static_cast<Players>(gMap[ny-2][nx+1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+1, ny-2);
                }
                if ((nx+2) < 8 && (ny-1) >= 0 && (static_cast<Players>(gMap[ny-1][nx+2]) >= Players::B_KING || static_cast<Players>(gMap[ny-1][nx+2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+2, ny-1);
                }
                if ((nx+2) < 8 && (ny+1) < 8 && (static_cast<Players>(gMap[ny+1][nx+2]) >= Players::B_KING || static_cast<Players>(gMap[ny+1][nx+2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+2, ny+1);
                }
                if ((nx+1) < 8 && (ny+2) < 8 && (static_cast<Players>(gMap[ny+2][nx+1]) >= Players::B_KING || static_cast<Players>(gMap[ny+2][nx+1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+1, ny+2);
                }

                if ((nx-1) >= 0 && (ny+2) < 8 && (static_cast<Players>(gMap[ny+2][nx-1]) >= Players::B_KING || static_cast<Players>(gMap[ny+2][nx-1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-1, ny+2);
                }
                if ((nx-2) >= 0 && (ny+1) < 8 && (static_cast<Players>(gMap[ny+1][nx-2]) >= Players::B_KING || static_cast<Players>(gMap[ny+1][nx-2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-2, ny+1);
                }
                if ((nx-2) >= 0 && (ny-1) >= 0 && (static_cast<Players>(gMap[ny-1][nx-2]) >= Players::B_KING || static_cast<Players>(gMap[ny-1][nx-2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-2, ny-1);
                }
                if ((nx-1) >= 0 && (ny-2) >= 0 && (static_cast<Players>(gMap[ny-2][nx-1]) >= Players::B_KING || static_cast<Players>(gMap[ny-2][nx-1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-1, ny-2);
                }

                break;
            }
            case Players::B_KNIGHT: {

                int nx = x;
                int ny = y;

                if ((nx+1) < 8 && (ny-2) >= 0 && ((static_cast<Players>(gMap[ny-2][nx+1]) >= Players::W_KING && static_cast<Players>(gMap[ny-2][nx+1]) < Players::B_KING) || static_cast<Players>(gMap[ny-2][nx+1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+1, ny-2);
                }
                if ((nx+2) < 8 && (ny-1) >= 0 && ((static_cast<Players>(gMap[ny-1][nx+2]) >= Players::W_KING && static_cast<Players>(gMap[ny-1][nx+2]) < Players::B_KING) || static_cast<Players>(gMap[ny-1][nx+2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+2, ny-1);
                }
                if ((nx+2) < 8 && (ny+1) < 8 && ((static_cast<Players>(gMap[ny+1][nx+2]) >= Players::W_KING && static_cast<Players>(gMap[ny+1][nx+2]) < Players::B_KING) || static_cast<Players>(gMap[ny+1][nx+2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+2, ny+1);
                }
                if ((nx+1) < 8 && (ny+2) < 8 && ((static_cast<Players>(gMap[ny+2][nx+1]) >= Players::W_KING && static_cast<Players>(gMap[ny+2][nx+1]) < Players::B_KING) || static_cast<Players>(gMap[ny+2][nx+1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx+1, ny+2);
                }

                if ((nx-1) >= 0 && (ny+2) < 8 && ((static_cast<Players>(gMap[ny+2][nx-1]) >= Players::W_KING && static_cast<Players>(gMap[ny+2][nx-1]) < Players::B_KING) || static_cast<Players>(gMap[ny+2][nx-1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-1, ny+2);
                }
                if ((nx-2) >= 0 && (ny+1) < 8 && ((static_cast<Players>(gMap[ny+1][nx-2]) >= Players::W_KING && static_cast<Players>(gMap[ny+1][nx-2]) < Players::B_KING) || static_cast<Players>(gMap[ny+1][nx-2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-2, ny+1);
                }
                if ((nx-2) >= 0 && (ny-1) >= 0 && ((static_cast<Players>(gMap[ny-1][nx-2]) >= Players::W_KING && static_cast<Players>(gMap[ny-1][nx-2]) < Players::B_KING) || static_cast<Players>(gMap[ny-1][nx-2]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-2, ny-1);
                }
                if ((nx-1) >= 0 && (ny-2) >= 0 && ((static_cast<Players>(gMap[ny-2][nx-1]) >= Players::W_KING && static_cast<Players>(gMap[ny-2][nx-1]) < Players::B_KING) || static_cast<Players>(gMap[ny-2][nx-1]) == Players::EMPTY)){
                    possibleMoves.emplace_back(nx-1, ny-2);
                }

                break;
            }

            case Players::W_PAWN: {
                // gMap[y][x] ueberpruefen, ob die jeweiligen Felder zum spielen frei sind (0)
                std::cout << "W Pawn" << std::endl;
                if (y > 0) {
                    if (static_cast<Players>(gMap[y-1][x]) == Players::EMPTY) {
                        std::cout << "Feld frei" << std::endl;
                        possibleMoves.emplace_back(x, y-1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 6) {
                            possibleMoves.emplace_back(x, y-2);
                        }
                    }
                    if (x > 0 && static_cast<Players>(gMap[y-1][x-1]) != Players::EMPTY) {
                        if (static_cast<Players>(gMap[y-1][x-1]) >= Players::W_KING && static_cast<Players>(gMap[y-1][x-1]) < Players::B_KING){
                            break;
                        }
                        possibleMoves.emplace_back(x-1, y-1);
                    }
                    if (x < 7 && static_cast<Players>(gMap[y-1][x+1]) != Players::EMPTY) {
                        if (static_cast<Players>(gMap[y-1][x+1]) >= Players::W_KING && static_cast<Players>(gMap[y-1][x+1]) < Players::B_KING){
                            break;
                        }
                        possibleMoves.emplace_back(x+1, y-1);
                    }
                }
                break;
            }
            case Players::B_PAWN: {
                // gMap[y][x] ueberpruefen, ob die jeweiligen Felder zum spielen frei sind (0)
                std::cout << "B Pawn" << std::endl;
                if (y < 7) {
                    if (static_cast<Players>(gMap[y+1][x]) == Players::EMPTY) {
                        std::cout << "Feld frei" << std::endl;
                        possibleMoves.emplace_back(x, y+1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 1) {
                            possibleMoves.emplace_back(x, y+2);
                        }
                    }
                    if (x > 0 && static_cast<Players>(gMap[y+1][x-1]) != Players::EMPTY) {
                        if (static_cast<Players>(gMap[y+1][x-1]) >= Players::B_KING){
                            break;
                        }
                        possibleMoves.emplace_back(x-1, y+1);
                    }
                    if (x < 7 && static_cast<Players>(gMap[y+1][x+1]) != Players::EMPTY) {
                        if (static_cast<Players>(gMap[y+1][x+1]) >= Players::B_KING){
                            break;
                        }
                        possibleMoves.emplace_back(x+1, y+1);
                    }
                }
                break;
            }
            case Players::EMPTY:
            default:
                std::cout << "No valid move" << std::endl;
                break;
        }



        // Output = Anzeige aller moeglichen Felder
    }

    void drawPossibleMoves() {
        for (auto move : possibleMoves) {
            float nx = this->fieldX + (blocksize * move.first);
            float ny = this->fieldY + (blocksize * move.second);
            DrawRectangle(nx, ny, blocksize, blocksize, {140,0,0,255});
            DrawRectangleLines(nx, ny, blocksize, blocksize, BLACK);
        }
    }


};




int main() {

    const int screenW = 1200;
    const int screenH = 1000;

    InitWindow(screenW, screenH, "chess");

    Camera2D cam = {0};
    cam.target = Vector2{ screenW / 2.0f, screenH / 2.0f };
    cam.offset = Vector2{ screenW / 2.0f, screenH / 2.0f };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BROWN);

        BeginMode2D(cam);

        game.update();
        game.draw();

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}