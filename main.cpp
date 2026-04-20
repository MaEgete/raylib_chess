#include <iostream>
#include "raylib.h"
#include <vector>
#include <utility>

class Game {

private:

    //Figuren verschieben koennen
    //-> Brett geraeusch erklingen lassen

    //Funktion isPlayerWhite/Black() schreiben

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

    // Texture des Bildes der Schachfiguren
    Texture2D pieceSpritesheet;

    // Fuer die Bilder der Figuren
    enum class PieceSprite {
        W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_KING, W_QUEEN,
        B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_KING, B_QUEEN,
        COUNT
    };

    // Hier werden die Rectangles der Texture gespeichert
    std::vector<Rectangle> pieceSprites{static_cast<int>(PieceSprite::COUNT)};

    int gMap[8][8] = {
        {9 ,11,10,8 ,7 ,10,11,9},
        {12 ,12 ,12 ,12 ,12 ,12 ,12 ,12},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {6 ,6 ,6 ,6 ,6 ,6 ,6 ,6},
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



    void drawFigures() {

        float scale = 2.5f;

        auto drawFiguresHelpMethod = [&](PieceSprite pieceSprite, int nx, int ny) {
            Rectangle src = pieceSprites[static_cast<int>(pieceSprite)];
            auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
            DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        };

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
                        drawFiguresHelpMethod(PieceSprite::W_KING, nx, ny);
                        break;
                    }
                    case Players::W_QUEEN: {
                        drawFiguresHelpMethod(PieceSprite::W_QUEEN, nx, ny);
                        break;
                    }
                    case Players::W_ROOK: {
                        drawFiguresHelpMethod(PieceSprite::W_ROOK, nx, ny);
                        break;
                    }
                    case Players::W_BISHOP: {
                        drawFiguresHelpMethod(PieceSprite::W_BISHOP, nx, ny);
                        break;
                    }
                    case Players::W_KNIGHT: {
                        drawFiguresHelpMethod(PieceSprite::W_KNIGHT, nx, ny);
                        break;
                    }
                    case Players::W_PAWN: {
                        drawFiguresHelpMethod(PieceSprite::W_PAWN, nx, ny);
                        break;
                    }

                    case Players::B_KING: {
                        drawFiguresHelpMethod(PieceSprite::B_KING, nx, ny);
                        break;
                    }
                    case Players::B_QUEEN: {
                        drawFiguresHelpMethod(PieceSprite::B_QUEEN, nx, ny);
                        break;
                    }
                    case Players::B_ROOK: {
                        drawFiguresHelpMethod(PieceSprite::B_ROOK, nx, ny);
                        break;
                    }
                    case Players::B_BISHOP: {
                        drawFiguresHelpMethod(PieceSprite::B_BISHOP, nx, ny);
                        break;
                    }
                    case Players::B_KNIGHT: {
                        drawFiguresHelpMethod(PieceSprite::B_KNIGHT, nx, ny);
                        break;
                    }
                    case Players::B_PAWN: {
                        drawFiguresHelpMethod(PieceSprite::B_PAWN, nx, ny);
                        break;
                    }
                    default:
                        std::cout << "False field value" << std::endl;
                        break;
                }

            }
        }
    }



    // Returns False when player is an empty grid
    bool isEnemyPiece(const Players& player) {

        if (isBlackPiece(player)) {
            // Weiss ist am Zug
            if (turn) {
                return true;
            }
        }
        else if (isWhitePiece(player)) {
            // Schwarz ist am Zug
            if (!turn) {
                return true;
            }
        }

        return false;
    }

    bool isEmpty(const Players& player) {
        return player == Players::EMPTY;
    }

    // Returns True when player is a black piece
    bool isBlackPiece(const Players& player){
        return player >= Players::B_KING;
    }

    bool isWhitePiece(const Players& player) {
        return player >= Players::W_KING && player < Players::B_KING;
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
            // Maus hat aufs Spielfeld geklickt
            if (mousePos.x > fieldX && mousePos.x < (fieldX + fieldlength)
                && mousePos.y > fieldY && mousePos.y < (fieldY + fieldlength)) {
                std::cout << "Mouse clicked on field" << std::endl;

                // Identifizieren des geklickten Blocks
                float nx = mousePos.x - static_cast<float>(fieldX);
                float ny = mousePos.y - static_cast<float>(fieldY);

                int x = static_cast<int>(nx / blocksize);
                int y = static_cast<int>(ny / blocksize);

                std::cout << "x: " << x << " y: " << y << std::endl;

                // Wenn das angeklickte Feld, schon angeklickt war, dann wird die Markierung weggemacht
                if (this->clickedField.first == x && this->clickedField.second == y) {
                    this->clickedField = {-1, -1};
                    std::cout << "---\nclicked field already clicked\n---" << std::endl;
                }

                // Wenn schon ein Feld markiert ist (ungleich -1,-1),
                else if (this->clickedField.first != -1 && this->clickedField.second != -1) {
                    // und das neu angeklickte Feld Teil von possibleMoves ist (dunkelrote Bloecke)
                    bool found = false;
                    for (auto& move : possibleMoves) {
                        if (move.first == x && move.second == y) {
                            std::cout << "---\nclicked possible move\n---" << std::endl;


                            found = true;

                            // Endposition wird auf die Spielerindex gesetzt
                            gMap[move.second][move.first] = static_cast<int>(gMap[clickedField.second][clickedField.first]);

                            // Ursprungsposition wird auf 0 gesetzt (EMPTY)
                            gMap[clickedField.second][clickedField.first] = static_cast<int>(Players::EMPTY);

                            // Markierung nach erfolgreichem Zug wegmachen
                            this->clickedField = {-1, -1};
                        }
                    }

                    // Wenn kein possibleMove angeklickt wurde, dann wird stattdessen ein neues Feld markiert
                    if (found == false) {
                        this->clickedField = {x, y};
                        std::cout << "---\nclicked new field2\n---" << std::endl;
                    }
                }

                // Wenn Feld geklickt wurde, und davor war noch keins markiert
                else {
                    // Neues angeklicktes Feld
                    this->clickedField = {x, y};
                    std::cout << "---\nclicked new field\n---" << std::endl;
                }


                calculatePossibleMoves();


            }
            // Maus hat nicht auf das Spielfeld geklickt
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

                auto tryMove = [&](int dx, int dy) {

                    int nx = x;
                    int ny = y;

                    nx += dx;
                    ny += dy;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        return;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    // Wenn Feld leer ist oder auf dem Feld eine Schwarze Figur steht
                    if (isEmpty(player) || isBlackPiece(player)) {
                        // Move hinzufuegen
                        possibleMoves.emplace_back(nx, ny);
                    }

                    //if (player == Players::EMPTY || player >= Players::B_KING) {
                    //    possibleMoves.emplace_back(nx, ny);
                    //}
                };

                tryMove(0, -1);
                tryMove(1, -1);
                tryMove(1, 0);
                tryMove(1, 1);
                tryMove(0, 1);
                tryMove(-1, 1);
                tryMove(-1, 0);
                tryMove(-1, -1);

                break;
            }
            case Players::B_KING: {

                auto tryMove = [&](int dx, int dy) {

                    int nx = x;
                    int ny = y;

                    nx += dx;
                    ny += dy;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        return;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    if (isEmpty(player) || isWhitePiece(player)) {
                        possibleMoves.emplace_back(nx, ny);
                    }

                    //if (player == Players::EMPTY || (player >= Players::W_KING && player < Players::B_KING)) {
                    //    possibleMoves.emplace_back(nx, ny);
                    //}
                };

                tryMove(0, -1);
                tryMove(1, -1);
                tryMove(1, 0);
                tryMove(1, 1);
                tryMove(0, 1);
                tryMove(-1, 1);
                tryMove(-1, 0);
                tryMove(-1, -1);


                break;
            }

            case Players::W_QUEEN: {

                auto tryMove = [&](int dx, int dy) {
                    int nx = x;
                    int ny = y;

                    while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                        nx += dx;
                        ny += dy;

                        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                            return;
                        }

                        auto player = static_cast<Players>(gMap[ny][nx]);

                        if (isEmpty(player)) {
                            possibleMoves.emplace_back(nx, ny);

                        }
                        else if (isWhitePiece(player)) {
                            break;
                        }
                        else if (isBlackPiece(player)) {
                            possibleMoves.emplace_back(nx, ny);
                            break;
                        }
                    }
                };

                tryMove(0, -1);
                tryMove(1, -1);
                tryMove(1, 0);
                tryMove(1, 1);
                tryMove(0, 1);
                tryMove(-1, 1);
                tryMove(-1, 0);
                tryMove(-1, -1);


                break;
            }
            case Players::B_QUEEN: {

                auto tryMove = [&](int dx, int dy) {
                    int nx = x;
                    int ny = y;

                    while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                        nx += dx;
                        ny += dy;

                        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                            return;
                        }

                        auto player = static_cast<Players>(gMap[ny][nx]);

                        if (isEmpty(player)) {
                            possibleMoves.emplace_back(nx, ny);
                        }
                        else if (isBlackPiece(player)) {
                            break;
                        }
                        else if (isWhitePiece(player)) {
                            possibleMoves.emplace_back(nx, ny);
                            break;
                        }
                    }
                };

                tryMove(0, -1);
                tryMove(1, -1);
                tryMove(1, 0);
                tryMove(1, 1);
                tryMove(0, 1);
                tryMove(-1, 1);
                tryMove(-1, 0);
                tryMove(-1, -1);


                break;
            }

            case Players::W_ROOK: {
                std::cout << "W Rook" << std::endl;

                // While-Schleife um zu ueberpruefen, wie viele Felder frei sind
                // Richtung nach oben
                int nx = x, ny = y;
                while (ny >= 0){
                    nx = x;
                    ny -= 1;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);


                    if (isWhitePiece(player) || ny < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isBlackPiece(player)) {
                        break;
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;
                while (ny <= 7){
                    nx = x;
                    ny += 1;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);


                    if ((isWhitePiece(player)) || ny > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isBlackPiece(player)) {
                        break;
                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;
                while (nx >= 0){
                    nx -= 1;
                    ny = y;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);


                    if ((isWhitePiece(player)) || nx < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isBlackPiece(player)) {
                        break;
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;
                while (nx >= 0){
                    nx += 1;
                    ny = y;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);


                    if ((isWhitePiece(player)) || nx > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isBlackPiece(player)) {
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
                while (ny >= 0){
                    nx = x;
                    ny -= 1;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    if ((isBlackPiece(player)) || ny < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isWhitePiece(player)) {
                        break;
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;
                while (ny <= 7){
                    nx = x;
                    ny += 1;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    if ((isBlackPiece(player)) || ny > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isWhitePiece(player)) {
                        break;
                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;
                while (nx >= 0){
                    nx -= 1;
                    ny = y;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    if ((isBlackPiece(player)) || nx < 0){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isWhitePiece(player)) {
                        break;
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;
                while (nx >= 0){
                    nx += 1;
                    ny = y;

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                        break;
                    }

                    auto player = static_cast<Players>(gMap[ny][nx]);

                    if ((isBlackPiece(player)) || nx > 7){
                        break;
                    }
                    possibleMoves.emplace_back(nx, ny);
                    if (isWhitePiece(player)) {
                        break;
                    }
                }

                break;
            }

            case Players::W_BISHOP: {

                int startX = x;
                int startY = y;

                auto tryMove = [&](int dx, int dy) {
                    int nx = startX;
                    int ny = startY;

                    while (true) {
                        nx += dx;
                        ny += dy;

                        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                            break;
                        }

                        auto player = static_cast<Players>(gMap[ny][nx]);

                        // Weiss darf weiss nicht schlagen
                        if (isWhitePiece(player)) {
                            break;
                        }

                        // Weiss darf Schwarz schlagen
                        if (isBlackPiece(player)) {
                            possibleMoves.emplace_back(nx, ny);
                            break;
                        }
                        // Leeres Feld ist begehbar
                        if (isEmpty(player)) {
                            possibleMoves.emplace_back(nx, ny);
                        }

                    }

                };

                tryMove(1,-1); // oben rechts
                tryMove(-1,-1); // oben links
                tryMove(1,1); // unten rechts
                tryMove(-1, 1); // unten links

                break;
            }
            case Players::B_BISHOP: {


                int startX = x;
                int startY = y;

                auto tryMove = [&](int dx, int dy) {
                    int nx = startX;
                    int ny = startY;

                    while (true) {
                        nx += dx;
                        ny += dy;

                        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
                            break;
                        }

                        auto player = static_cast<Players>(gMap[ny][nx]);

                        // Schwarz darf schwarz nicht schlagen
                        if (isBlackPiece(player)) {
                            break;
                        }
                        // Schwarz darf weiss schlagen
                        if (isWhitePiece(player)) {
                            possibleMoves.emplace_back(nx, ny);
                            break;
                        }
                        // Leeres Feld ist begehbar
                        if (isEmpty(player)) {
                            possibleMoves.emplace_back(nx, ny);
                        }

                    }

                };

                tryMove(1,-1); // oben rechts
                tryMove(-1,-1); // oben links
                tryMove(1,1); // unten rechts
                tryMove(-1, 1); // unten links


                break;
            }

            case Players::W_KNIGHT: {
                int nx = x;
                int ny = y;

                auto tryMove = [&](int tx, int ty) {
                    if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                        auto player = static_cast<Players>(gMap[ty][tx]);
                        if (isEmpty(player) || isBlackPiece(player)) {
                            possibleMoves.emplace_back(tx, ty);
                        }
                    }
                };

                tryMove(nx+1, ny-2);
                tryMove(nx+2, ny-1);
                tryMove(nx+2, ny+1);
                tryMove(nx+1, ny+2);
                tryMove(nx-1, ny+2);
                tryMove(nx-2, ny+1);
                tryMove(nx-2, ny-1);
                tryMove(nx-1, ny-2);


                break;
            }
            case Players::B_KNIGHT: {

                int nx = x;
                int ny = y;


                auto tryMove = [&](int tx, int ty) {
                    if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                        auto player = static_cast<Players>(gMap[ty][tx]);
                        if (isEmpty(player) || (isWhitePiece(player))) {
                            possibleMoves.emplace_back(tx, ty);
                        }
                    }
                };

                tryMove(nx+1, ny-2);
                tryMove(nx+2, ny-1);
                tryMove(nx+2, ny+1);
                tryMove(nx+1, ny+2);
                tryMove(nx-1, ny+2);
                tryMove(nx-2, ny+1);
                tryMove(nx-2, ny-1);
                tryMove(nx-1, ny-2);

                break;
            }

            case Players::W_PAWN: {
                // gMap[y][x] ueberpruefen, ob die jeweiligen Felder zum spielen frei sind (0)
                std::cout << "W Pawn" << std::endl;
                if (y > 0) {
                    // Wenn Feld frei ist
                    if (isEmpty(static_cast<Players>(gMap[y-1][x]))) {
                        std::cout << "Feld frei" << std::endl;
                        possibleMoves.emplace_back(x, y-1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 6 && !isWhitePiece(static_cast<Players>(gMap[y-2][x]))) {
                            possibleMoves.emplace_back(x, y-2);
                        }
                    }
                    // Wenn das Feld links oben links nicht leer ist
                    if (x > 0 && !isEmpty(static_cast<Players>(gMap[y-1][x-1]))) {
                        // Wenn auf dem Feld eine weisse Schachfigur ist
                        if (isWhitePiece(static_cast<Players>(gMap[y-1][x-1]))){

                        }
                        // Wenn auf dem Feld eine schwarze Schachfigur ist
                        else {
                            possibleMoves.emplace_back(x-1, y-1);
                        }
                    }
                    // Wenn das Feld oben rechts nicht leer ist
                    if (x < 7 && !isEmpty(static_cast<Players>(gMap[y-1][x+1]))) {
                        // Wenn auf dem Feld eine weisse Schachfigur ist
                        if (isWhitePiece(static_cast<Players>(gMap[y-1][x+1]))){
                        }
                        // Wenn auf dem Feld eine schwarze Schachfigur ist
                        else {
                            possibleMoves.emplace_back(x+1, y-1);
                        }
                    }
                }
                break;
            }
            case Players::B_PAWN: {
                // gMap[y][x] ueberpruefen, ob die jeweiligen Felder zum spielen frei sind (0)
                std::cout << "B Pawn" << std::endl;
                if (y < 7) {
                    if (isEmpty(static_cast<Players>(gMap[y+1][x]))) {
                        std::cout << "Feld frei" << std::endl;
                        possibleMoves.emplace_back(x, y+1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 1 && !isBlackPiece(static_cast<Players>(gMap[y+2][x]))) {
                            possibleMoves.emplace_back(x, y+2);
                        }
                    }
                    // Richtung unten links
                    // Wenn unten links ein Spieler ist
                    if (x > 0 && !isEmpty(static_cast<Players>(gMap[y+1][x-1]))) {
                        // Wenn die Figur unten links Schwarz ist, dann soll nichts gemacht werden
                        if (isBlackPiece(static_cast<Players>(gMap[y+1][x-1]))){
                        }
                        // Wenn der Spieler unten links ein Weisser ist
                        else {
                            possibleMoves.emplace_back(x-1, y+1);
                        }
                    }
                    if (x < 7 && !isEmpty(static_cast<Players>(gMap[y+1][x+1]))) {
                        if (isBlackPiece(static_cast<Players>(gMap[y+1][x+1]))){
                        }
                        else {
                            possibleMoves.emplace_back(x+1, y+1);
                        }
                    }
                }
                break;
            }

            case Players::EMPTY:
            default:
                std::cout << "No valid move" << std::endl;
                break;
        }

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