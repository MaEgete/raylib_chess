#include <iostream>
#include "raylib.h"
#include <vector>
#include <utility>
#include <algorithm>
#include <complex>
#include <ranges>
#include <format>

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

    // Restart button
    int restartX;
    int restartY;
    int restartW;
    int restartH;

    // PLAY = Spielen
    // CHOOSE_MODE = Figur austauschen mit Bauer
    enum class GameMode {
        PLAY,
        CHOOSE_MODE,
        END,
    };

    enum class Won {
        WON_WHITE,
        WON_BLACK,
        NONE,
    };

    enum class Check {
        CHECK_WHITE,CHECK_BLACK,
        CHECK_NONE,
    };

    GameMode gameMode = GameMode::PLAY;

    Won won = Won::NONE;
    Check check = Check::CHECK_NONE;

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



    std::vector<Players> lostBlackPieces{};
    std::vector<Players> lostWhitePieces{};
    std::vector<Players> allPieces{Players::W_KING, Players::W_QUEEN, Players::W_ROOK, Players::W_BISHOP,
        Players::W_KNIGHT, Players::W_PAWN, Players::B_KING, Players::B_QUEEN, Players::B_ROOK, Players::B_BISHOP,
        Players::B_KNIGHT, Players::B_PAWN};

    // Fuer Choose
    std::vector<Players> allWhitePieces{Players::W_QUEEN, Players::W_ROOK, Players::W_BISHOP,
        Players::W_KNIGHT};

    std::vector<Players> allBlackPieces{Players::B_QUEEN, Players::B_ROOK, Players::B_BISHOP,
        Players::B_KNIGHT};

    std::pair<int, int> lastMove = {-1, -1};

    int chooseRecW = 100;
    int chooseRecH = 30;

    std::vector<std::pair<int, int>> allPossbileMovesFromEnemy;

    std::vector<std::pair<int, int>> theoreticalMoves;

    std::pair<int, int> whiteKingPosition = {-1, -1};
    std::pair<int, int> blackKingPosition = {-1, -1};

    std::vector<std::pair<int, int>> whiteKingPossibleMoves{};
    std::vector<std::pair<int, int>> blackKingPossibleMoves{};


    static bool whiteKingMoved;
    static bool blackKingMoved;
    static bool whiteRightRookMoved;
    static bool blackRightRookMoved;
    static bool whiteLeftRookMoved;
    static bool blackLeftRookMoved;


    Rectangle logRectangle;

    // Eine Liste, wo einem Spieler ein Zug zugewiesen wird
    std::vector<std::pair<Players, std::pair<int,int>>> logList;



public:

    Game(int fieldlength = 600) : fieldlength{fieldlength}, blocksize{fieldlength / 8} {
        this->midX = GetScreenWidth() / 2;
        this->midY = GetScreenHeight() / 2;

        fieldX = midX - (fieldlength / 2);
        fieldY = midY - (fieldlength / 2);

        field = Rectangle{static_cast<float>(fieldX), static_cast<float>(fieldY), static_cast<float>(fieldlength), static_cast<float>(fieldlength)};

        restartX = this->fieldX + fieldlength/4;
        restartY = this->fieldY + fieldlength + 80;
        restartW = fieldlength/2;
        restartH = 150;

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

        updateKingPosition();

        logRectangle = Rectangle{(float)this->fieldX, 10, (float)this->fieldlength, 180};

    }

    ~Game() {
        UnloadTexture(pieceSpritesheet);
    }

    bool mouseCollision(Vector2 mousePos, Rectangle rec){

        if (mousePos.x >= rec.x && mousePos.x <= (rec.x + rec.width) &&
            mousePos.y >= rec.y && mousePos.y <= (rec.y + rec.height)) \
        {
            return true;
        }
        return false;
    }

    void loop() {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            // Maus hat auf Restart geklickt
            if (mousePos.x >= restartX && mousePos.x <= (restartX + restartW)
                && mousePos.y >= restartY && mousePos.y <= (restartY + restartH)) {

                std::cout << "Restart" << std::endl;
                restart();
                }
        }

        if (gameMode == GameMode::PLAY) {
            update();
        }
        else if (gameMode == GameMode::CHOOSE_MODE){
            // Wenn die Maus eine Figur ausgewaehlt hat
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                std::cout << "test" << std::endl;
                //Hier irgendwie dann die Figuren aus der Liste auswaehlen koennen
                //Auflistung ist so:

                // Die fontSize 30 ist die hoehe von den Feldern
                int fontSize = this->chooseRecH;
                int xPos = this->fieldX + 10;
                int yPos = this->fieldY + 10 + 40 + fontSize;

                for (int i = 0; i < allWhitePieces.size(); i++) {
                    Rectangle rec(xPos, yPos, this->chooseRecW, this->chooseRecH);
                    if (mouseCollision(mousePos, rec)) {
                        std::cout << "mouseCollision" << std::endl;
                        std::cout << "i = " << i << std::endl;

                        switch (i) {
                            case 0:
                                std::cout << "QUEEN" << std::endl;

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_QUEEN);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_QUEEN);
                                }
                                break;
                            case 1:
                                std::cout << "ROOK" << std::endl;

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_ROOK);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_ROOK);
                                }
                                break;
                            case 2:
                                std::cout << "BISHOP" << std::endl;

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_BISHOP);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_BISHOP);
                                }

                                break;
                            case 3:
                                std::cout << "KNIGHT" << std::endl;

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_KNIGHT);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_KNIGHT);
                                }
                                break;

                        }

                        // i == 0 -> KING
                        // i == 1 -> QUEEN
                        // i == 2 -> ROOK
                        // i == 3 -> BISHOP
                        // i == 4 -> KNIGHT

                        gameMode = GameMode::PLAY;

                    }

                    yPos += fontSize;
                }


            }

        }
        else {

        }
        draw();
    }

    // Spiel zeichnen
    void draw() {
        drawField();
        drawClickedField();
        drawFigures();
        drawText();
        drawLostFigures();
        drawLogList();

        if (gameMode == GameMode::CHOOSE_MODE) {
            drawChooseField();
        }

        if (gameMode == GameMode::END) {
            drawEndField();
        }

        if (this->check != Check::CHECK_NONE) {
            if (this->check == Check::CHECK_WHITE) {
                DrawText("Schach", this->fieldX, this->fieldY - 100, 100, WHITE);
            }
            if (this->check == Check::CHECK_BLACK) {
                DrawText("Schach", this->fieldX, this->fieldY - 100, 100, BLACK);

            }
        }
        if (this->won != Won::NONE) {
            if (this->won == Won::WON_WHITE) {
                DrawText("Schachmatt!", this->fieldX, this->fieldY - 100, 100, WHITE);
                gameMode = GameMode::END;
            }
            if (this->won == Won::WON_BLACK) {
                DrawText("Schachmatt!", this->fieldX, this->fieldY - 100, 100, BLACK);
                gameMode = GameMode::END;
            }
        }

        DrawRectangle(restartX, restartY, restartW, restartH, WHITE);
        DrawRectangleLines(restartX+10, restartY + 10, restartW - 20, restartH - 20, BLACK);
        int fontSize = 50;
        int width = MeasureText("Restart!", fontSize);
        DrawText("Restart!", this->midX - width/2, this->restartY + restartH/2 - fontSize/2, fontSize, BLACK);
    }

    // Regeln
    void update() {
        rochade();
        moveFigure();
    }

    std::string playerToString(const Players& player) {
        switch (player) {
            case Players::W_KING:
                return "W_KING";
                break;
            case Players::W_QUEEN:
                return "W_QUEEN";
                break;
            case Players::W_ROOK:
                return "W_ROOK";
                break;
            case Players::W_BISHOP:
                return "W_BISHOP";
                break;
            case Players::W_KNIGHT:
                return "W_KNIGHT";
                break;
            case Players::W_PAWN:
                return "W_PAWN";
                break;
            case Players::B_KING:
                return "B_KING";
                break;
            case Players::B_QUEEN:
                return "B_QUEEN";
                break;
            case Players::B_ROOK:
                return "B_ROOK";
                break;
            case Players::B_BISHOP:
                return "B_BISHOP";
                break;
            case Players::B_KNIGHT:
                return "B_KNIGHT";
                break;
            case Players::B_PAWN:
                return "B_PAWN";
                break;
        }

        return "";
    }

    std::string coordinatesToLabels(int x, int y) {

        std::cout << "x: " << x << " y: " << y << std::endl;

        std::string text = "";
        switch (x) {
            case 0:
                text += "A";
                break;
            case 1:
                text += "B";
                break;
            case 2:
                text += "C";
                break;
            case 3:
                text += "D";
                break;
            case 4:
                text += "E";
                break;
            case 5:
                text += "F";
                break;
            case 6:
                text += "G";
                break;
            case 7:
                text += "H";
                break;
        }

        std::vector<int> tmp{8,7,6,5,4,3,2,1};

        text += std::to_string(tmp.at(y));

        std::cout << "location: " << text << std::endl;

        return text;

    }




    void drawLogList() {

        DrawRectangleRec(logRectangle, {153,152,92,150});

        DrawLine(logRectangle.x + logRectangle.width/2,
            logRectangle.y + 10,
            logRectangle.x + logRectangle.width/2,
            logRectangle.y + logRectangle.height - 10, \
            {110, 95, 0, 255});


        //DrawText("test", logRectangle.x + 10, logRectangle.y + 10, 30, BLACK);



        // Eintraege der Liste printen - Weiss links - Schwarz rechts
        int wCount = 0;
        int bCount = 0;
        for (const std::pair<Players, std::pair<int,int>>& var : logList) {
            Players player = var.first;
            std::string text = playerToString(player);

            int x = var.second.first;
            int y = var.second.second;

            std:: cout << "x: " << x << " y: " << y << std::endl;

            std::string location = coordinatesToLabels(x, y);

            text += " -> " + location;

            int fontSize = 30;

            // Weiss auf der linken Spalte
            if (isWhitePiece(player)) {
                DrawText(text.c_str(), logRectangle.x + 10, logRectangle.y + 10 + fontSize * wCount, fontSize, WHITE);
                wCount++;
            }
            // Schwarz auf der rechten Spalte
            else if (isBlackPiece(player)) {
                DrawText(text.c_str(), logRectangle.x + logRectangle.width / 2 + 10, logRectangle.y + 10 + fontSize * bCount, 30, BLACK);
                bCount++;
            }

        }



    }


    void drawEndField() {
        DrawRectangleRec(field, {200, 96, 232,200});

        if (won == Won::WON_WHITE) {
            std::string text = "Weiss hat gewonnen!";

            int fontSize = 30;
            int textWidth = MeasureText(text.c_str(), fontSize);

            DrawText(text.c_str(), field.x + (field.width / 2) - textWidth/2, field.y + (field.height / 2) - fontSize/2, fontSize, WHITE);
        }
        else if (won == Won::WON_BLACK) {
            std::string text = "Schwarz hat gewonnen!";

            int fontSize = 30;
            int textWidth = MeasureText(text.c_str(), fontSize);

            DrawText(text.c_str(), field.x + (field.width / 2) - textWidth/2, field.y + (field.height / 2) - fontSize/2, fontSize, BLACK);
        }


    }


    void drawChooseField() {

        // Blaues Feld
        DrawRectangleRec(field, {12, 96, 232,200});

        DrawText("Waehle eine Figur:", this->fieldX + 10, this->fieldY + 13, 30, BLACK);
        DrawText("Waehle eine Figur:", this->fieldX + 10, this->fieldY + 10, 30, RED);

        // Weiss ist am Zug
        if (!turn) {
            drawFiguresLost(allWhitePieces, 0, this->fieldX + 10, 30, 1, this->fieldY + 10 + 40, true, GRAY);
        }
        else {
            drawFiguresLost(allBlackPieces, this->fieldX + 10, 0, 30, 1, this->fieldY + 10 + 40, true, GRAY);
        }


    }

    void drawFiguresLost(std::vector<Players>& vect, int blackXoff = 0, int whiteXoff = 0, int fontSize = 30, int offset = 1, int yOff = 0, bool background = false, Color backgroundColor = GREEN) {
        for (auto& var : vect) {
            switch (var) {
                case Players::B_KING: {
                    //std::cout << "B_KING" << std::endl;
                    const char* text = "B_KING";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
                    }
                case Players::B_QUEEN: {
                    //std::cout << "B_QUEEN" << std::endl;
                    const char* text = "B_QUEEN";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
            }
                case Players::B_ROOK: {
                    //std::cout << "B_ROOK" << std::endl;
                    const char* text = "B_ROOK";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
                    }
                case Players::B_BISHOP: {
                    //std::cout << "B_BISHOP" << std::endl;
                    const char* text = "B_BISHOP";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
                    }
                case Players::B_KNIGHT: {
                    //std::cout << "B_KNIGHT" << std::endl;
                    const char* text = "B_KNIGHT";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
                    }
                case Players::B_PAWN: {
                    //std::cout << "B_PAWN" << std::endl;
                    const char* text = "B_PAWN";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(blackXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, blackXoff, yOff + fontSize * offset, fontSize, BLACK);
                    offset++;
                    break;
                    }
                case Players::W_KING: {
                    //std::cout << "W_KING" << std::endl;

                    const char* text = "W_KING";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }
                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;

                    break;
                    }
                case Players::W_QUEEN: {
                    //std::cout << "W_QUEEN" << std::endl;

                    const char* text = "W_QUEEN";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;
                    break;
                    }
                case Players::W_ROOK: {
                    //std::cout << "W_ROOK" << std::endl;
                    const char* text = "W_ROOK";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;
                    break;
                    }
                case Players::W_BISHOP: {
                    //std::cout << "W_BISHOP" << std::endl;
                    const char* text = "W_BISHOP";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;
                    break;
                    }
                case Players::W_KNIGHT: {
                    //std::cout << "W_KNIGHT" << std::endl;
                    const char* text = "W_KNIGHT";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;
                    break;
                    }
                case Players::W_PAWN: {
                    //std::cout << "W_PAWN" << std::endl;
                    const char* text = "W_PAWN";
                    int textWidth = MeasureText(text, fontSize);
                    if (background) {
                        DrawRectangle(whiteXoff, yOff + fontSize * offset, textWidth, fontSize, backgroundColor);
                    }

                    DrawText(text, whiteXoff, yOff + fontSize * offset, fontSize, WHITE);
                    offset++;
                    break;
                    }
            }
        }
    }

    //Wenn nur noch ein Spieler auf dem Feld ist und dieser keinen PossibleMove hat, dann Schachmatt!

    void drawLostFigures() {


        // Schwarze Figuren
        int fontSize = 30;
        DrawText("Verluste von Schwarz:", 10, this->fieldY - fontSize, fontSize, BLACK);

        int blackXoff = 10;

        int offset = 1;

        drawFiguresLost(lostBlackPieces, blackXoff, 0, fontSize, offset,  this->fieldY);

        int whiteXoff = this->fieldX + this->fieldlength + 30;

        DrawText("Verluste von Weiss:", whiteXoff, this->fieldY - fontSize, fontSize, WHITE);

        drawFiguresLost(lostWhitePieces, 0, whiteXoff, fontSize, offset,  this->fieldY);


    }



    void drawText() const {
        DrawText(TextFormat("%s to move", (this->turn ? "White" : "Black"), 20), 10, 10, 40, (this->turn ? WHITE : BLACK));
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

        drawLabels();


    }

    void drawLabels() {
        std::vector<std::string> labelsY = {"A", "B", "C", "D", "E", "F", "G", "H"};
        std::vector<int> labelsX = {8, 7, 6, 5, 4, 3, 2, 1};

        int fontsize = 30;
        int offset = 20;

        int i = 0;
        for (const auto& label : labelsY) {

            int textWidth = MeasureText(label.c_str(), fontsize);


            DrawText(label.c_str(), this->fieldX + blocksize/2 - textWidth/2 + i++ * blocksize, this->fieldY + fieldlength + offset, fontsize, BLACK);
        }

        i = 0;
        for (const auto& label : labelsX) {
            int textWidth = MeasureText(std::format("{}", label).c_str(), fontsize);
            DrawText(std::format("{}", label).c_str(), this->fieldX - textWidth - offset, this->fieldY + blocksize/2 + i++ * blocksize - fontsize/2, fontsize, BLACK);
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


    void restart() {

         int tmp[8][8] = {
            {9 ,11,10,8 ,7 ,10,11,9},
            {12 ,12 ,12 ,12 ,12 ,12 ,12 ,12},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {6 ,6 ,6 ,6 ,6 ,6 ,6 ,6},
            {3 ,5 ,4 ,2 ,1 ,4 ,5 ,3},
        };

        memcpy(gMap, tmp, sizeof(tmp));
        // Weiss ist am Zug
        turn = true;

        // Reset
        lostBlackPieces.clear();
        lostWhitePieces.clear();

        logList.clear();

        check = Check::CHECK_NONE;
        won = Won::NONE;
        gameMode = GameMode::PLAY;

        whiteKingMoved = false;
        blackKingMoved = false;
        whiteRightRookMoved = false;
        blackRightRookMoved = false;
        whiteLeftRookMoved = false;
        blackLeftRookMoved = false;


    }

    void updateKingPosition() {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (gMap[y][x] == static_cast<int>(Players::W_KING)) {
                    whiteKingPosition = {x, y};
                }
                else if (gMap[y][x] == static_cast<int>(Players::B_KING)) {
                    blackKingPosition = {x, y};
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
                            // Hier wird die Figur bewegt
                            std::cout << "---\nmoved figure\n---" << std::endl;

                            if (!isEmpty(static_cast<Players>(gMap[move.second][move.first]))) {
                                if (turn) {
                                    std::cout << "---\nBlack lost a figure\n---" << std::endl;
                                    lostBlackPieces.emplace_back(static_cast<Players>(gMap[move.second][move.first]));
                                }
                                else {
                                    std::cout << "---\nWhite lost a figure\n---" << std::endl;
                                    lostWhitePieces.emplace_back(static_cast<Players>(gMap[move.second][move.first]));
                                }
                            }

                            found = true;

                            // Weisser Koenig will kurze Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::W_KING) && move.second == 7 && move.first == 6) {
                                gMap[7][5] = static_cast<int>(Players::W_ROOK);
                                gMap[7][7] = static_cast<int>(Players::EMPTY);
                            }

                            // Schwarzer Koenig will kurze Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::B_KING) && move.second == 0 && move.first == 6) {
                                gMap[0][5] = static_cast<int>(Players::W_ROOK);
                                gMap[0][7] = static_cast<int>(Players::EMPTY);
                            }

                            // Weisser Koenig will lange Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::W_KING) && move.second == 7 && move.first == 2) {
                                gMap[7][3] = static_cast<int>(Players::W_ROOK);
                                gMap[7][0] = static_cast<int>(Players::EMPTY);
                            }

                            // Schwarzer Koenig will lange Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::W_KING) && move.second == 0 && move.first == 2) {
                                gMap[0][3] = static_cast<int>(Players::W_ROOK);
                                gMap[0][0] = static_cast<int>(Players::EMPTY);
                            }


                            // Endposition wird auf die Spielerindex gesetzt
                            gMap[move.second][move.first] = gMap[clickedField.second][clickedField.first];

                            Players player = static_cast<Players>(gMap[clickedField.second][clickedField.first]);

                            // Spielzug loggen
                            logList.emplace_back(player, move);


                            // Ursprungsposition wird auf 0 gesetzt (EMPTY)
                            gMap[clickedField.second][clickedField.first] = static_cast<int>(Players::EMPTY);

                            // Markierung nach erfolgreichem Zug wegmachen
                            this->clickedField = {-1, -1};

                            // Letzten Zug merken
                            lastMove.first = move.first;
                            lastMove.second = move.second;


                            // Wenn auf der obersten Linie ein weisser Bauer steht, dann soll das Fenster aufgerufen werden
                            // Das gleiche gilt fuer den schwarzen Bauern auf der untersten Linie

                            for (int i = 0; i < 8; i++) {
                                if (gMap[0][i] == static_cast<int>(Players::W_PAWN) || gMap[7][i] == static_cast<int>(Players::B_PAWN)) {
                                    gameMode = GameMode::CHOOSE_MODE;
                                }
                            }


                            // Spielerwechsel
                            turn = !turn;

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

    bool isThreatend(int x, int y, bool byWhite) {
        std::vector<std::pair<int, int>> enemyMoves;

        for (int yy = 0; yy < 8; yy++) {
            for (int xx = 0; xx < 8; xx++) {
                Players p = static_cast<Players>(gMap[yy][xx]);

                // Schwarz wird von Weiss bedroht
                // und Weiss wird von schwarz bedroht
                bool correctColor = (byWhite && isWhitePiece(p)) || (!byWhite && isBlackPiece(p));

                if (!correctColor) {
                    continue;
                }

                if (p == Players::W_KING || p == Players::B_KING) {
                    getKingAttackMoves(p, xx, yy, enemyMoves); // nur normale 1-Feld-Angriffe, keine Rochade
                }
                else if (p == Players::W_PAWN || p == Players::B_PAWN) {
                    getPawnAttackMoves(p, xx, yy, enemyMoves);
                }
                else {
                    getPossibleMoves(p, xx, yy, enemyMoves);
                }

            }
        }

        return std::find(enemyMoves.begin(), enemyMoves.end(), std::make_pair(x,y)) != enemyMoves.end();

    }


    bool wouldKingBeThreatenedAfterMove(int fromX, int fromY, int toX, int toY, Players king) {
        int captured = gMap[toY][toX];

        gMap[toY][toX] = static_cast<int>(king);
        gMap[fromY][fromX] = static_cast<int>(Players::EMPTY);

        bool threatened;

        if (king == Players::W_KING) {
            threatened = isThreatend(toX, toY, false); // von Schwarz bedroht
        }
        else {
            threatened = isThreatend(toX, toY, true); // von Weiss bedroht
        }

        gMap[fromY][fromX] = static_cast<int>(king);
        gMap[toY][toX] = captured;

        return threatened;
    }


    // Prüft, ob es IRGENDEINEN legalen Zug gibt,
    // der das aktuelle Schach aufhebt
    bool hasAnyLegalMoveToEscapeCheck(bool white) {

    // Gehe über das gesamte Brett
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            // Aktuelle Figur holen
            Players p = static_cast<Players>(gMap[y][x]);

            // Nur Figuren der aktuellen Farbe betrachten
            if (white && !isWhitePiece(p)) continue;
            if (!white && !isBlackPiece(p)) continue;

            // Alle möglichen Züge dieser Figur berechnen
            std::vector<std::pair<int,int>> moves;
            getPossibleMoves(p, x, y, moves);

            // Jeden möglichen Zug testen
            for (const auto& move : moves) {

                // Figur, die ggf. geschlagen wird, speichern
                int captured = gMap[move.second][move.first];

                // ---- ZUG SIMULIEREN ----

                // Figur auf Ziel setzen
                gMap[move.second][move.first] = gMap[y][x];

                // Ursprungsfeld leeren
                gMap[y][x] = static_cast<int>(Players::EMPTY);

                // König-Position neu bestimmen (wichtig!)
                updateKingPosition();

                // Prüfen, ob der eigene König danach noch im Schach ist
                bool stillInCheck = white
                    ? isThreatend(whiteKingPosition.first, whiteKingPosition.second, false)
                    : isThreatend(blackKingPosition.first, blackKingPosition.second, true);

                // ---- ZUG RÜCKGÄNGIG MACHEN ----

                // Ursprüngliche Figur zurücksetzen
                gMap[y][x] = static_cast<int>(p);

                // Geschlagene Figur (falls vorhanden) wiederherstellen
                gMap[move.second][move.first] = captured;

                // König-Position wieder aktualisieren
                updateKingPosition();

                // Wenn es EINEN Zug gibt, der das Schach aufhebt:
                if (!stillInCheck) {
                    return true; // Kein Schachmatt
                }
            }
        }
    }

    // Kein einziger Zug konnte das Schach verhindern → Schachmatt
    return false;
}



    void calculatePossibleMoves() {

        // Spieler(position) auf dem geklickten Feld
        int x = this->clickedField.first;
        int y = this->clickedField.second;


        if (x == -1 && y == -1) {
            std::cout << "No field clicked" << std::endl;
            return;
        }



        this->possibleMoves.clear();

        // Player
        auto player = static_cast<Players>(gMap[y][x]);

        allPossbileMovesFromEnemy.clear();
        theoreticalMoves.clear();
        // Komplettes Schachfeld durchlaufen und nach B_KING bis B_PAWN suchen
        // Davon dann die moeglichen Moves berechnen und in allPossbileMovesFromEnemy ablegen

        for (int newy = 0; newy < 8; newy++) {
            for (int newx = 0; newx < 8; newx++) {
                Players newPlayer = static_cast<Players>(gMap[newy][newx]);

                if (isWhitePiece(player)) {
                    if (isBlackPiece(newPlayer)) {
                        // Der Bauer hat einen Spezialfall. Er kann nur schraeg schlagen
                        if (newPlayer == Players::B_PAWN) {
                            getPawnAttackMoves(newPlayer, newx, newy, allPossbileMovesFromEnemy);
                        }
                        else {
                            getPossibleMoves(newPlayer, newx, newy, allPossbileMovesFromEnemy);
                        }
                    }
                }
                if (isBlackPiece(player)) {
                    if (isWhitePiece(newPlayer)) {
                        // Der Bauer hat einen Spezialfall. Er kann nur schraeg schlagen
                        if (newPlayer == Players::W_PAWN) {
                            getPawnAttackMoves(newPlayer, newx, newy, allPossbileMovesFromEnemy);
                        }
                        else {
                            getPossibleMoves(newPlayer, newx, newy, allPossbileMovesFromEnemy);
                        }
                    }
                }


            }
        }


        // Weiss ist am Zug
        if (turn && isBlackPiece(player)) {
            // Weiss kann nur weiss bedienen
            return;
        }

        // Schwarz ist am Zug
        if (!turn && isWhitePiece(player)) {
            // Schwarz kann nur schwarz bedienen
            return;
        }

        // Aufgrund des ermittelten Spielers, muessen die jeweiligen Move-Regeln herangezogen werden
        getPossibleMoves(player, x, y, possibleMoves);

        possibleMoves.erase(std::remove_if(possibleMoves.begin(), possibleMoves.end(), \
            [&](const std::pair<int, int>& move) {
                int captured = gMap[move.second][move.first];

                gMap[move.second][move.first] = gMap[y][x];
                gMap[y][x] = static_cast<int>(Players::EMPTY);

                updateKingPosition();

                bool ownKingInCheck;

                if (isWhitePiece(player)) {
                    ownKingInCheck = isThreatend(
                        whiteKingPosition.first,
                        whiteKingPosition.second,
                        false
                        );
                }
                else {
                    ownKingInCheck = isThreatend(
                        blackKingPosition.first,
                        blackKingPosition.second,
                        true
                        );
                }

                gMap[y][x] = static_cast<int>(player);
                gMap[move.second][move.first] = captured;

                updateKingPosition();

                return ownKingInCheck;
        }),
        possibleMoves.end());




        updateKingPosition();


        bool whiteInCheck = isThreatend(whiteKingPosition.first, whiteKingPosition.second, false);

        bool blackInCheck = isThreatend(blackKingPosition.first, blackKingPosition.second, true);



        if (whiteInCheck) {
            check = Check::CHECK_BLACK;
            won = Won::NONE;

            whiteKingPossibleMoves.clear();

            getPossibleMoves(Players::W_KING, whiteKingPosition.first, whiteKingPosition.second, whiteKingPossibleMoves);

            // Schauen ob die moeglichen Moves vom Koenig bedroht werden
            whiteKingPossibleMoves.erase(
                std::remove_if(whiteKingPossibleMoves.begin(), whiteKingPossibleMoves.end(),
                    [&](const std::pair<int, int>& move) {
                        return isThreatend(move.first, move.second, false);
                }),
                whiteKingPossibleMoves.end()
            );
            // Schwarz hat gewonnen
            if (!hasAnyLegalMoveToEscapeCheck(true)) {
                    check = Check::CHECK_NONE;
                    won = Won::WON_BLACK;
            }
        } else if (blackInCheck) {
            check = Check::CHECK_WHITE;
            won = Won::NONE;

            blackKingPossibleMoves.clear();

            getPossibleMoves(Players::B_KING, blackKingPosition.first, blackKingPosition.second, blackKingPossibleMoves);

            blackKingPossibleMoves.erase(
                std::remove_if(blackKingPossibleMoves.begin(), blackKingPossibleMoves.end(),
                    [&](const std::pair<int, int>& move) {
                        return isThreatend(move.first, move.second, true);
                    }),
                    blackKingPossibleMoves.end()
                );

            // Weiss hat gewonnen
            if (!hasAnyLegalMoveToEscapeCheck(false)) {
                    check = Check::CHECK_NONE;
                    won = Won::WON_WHITE;
            }
        } else {
            check = Check::CHECK_NONE;
            won = Won::NONE;
        }


        if (player == Players::W_KING || player == Players::B_KING) {

            // Schauen, ob Werte von possibleMoves uebereinstimmen mit Werten von allPossbileMovesFromEnemy
            possibleMoves.erase(
                std::remove_if(possibleMoves.begin(), possibleMoves.end(),[&](const std::pair<int, int>& move) {
                    return std::find(
                    allPossbileMovesFromEnemy.begin(),
                    allPossbileMovesFromEnemy.end(),
                    move) != allPossbileMovesFromEnemy.end();
                }),
                possibleMoves.end()
                );

            possibleMoves.erase(
                std::remove_if(possibleMoves.begin(), possibleMoves.end(),[&](const std::pair<int, int>& move) {
                    return std::find(
                    theoreticalMoves.begin(),
                    theoreticalMoves.end(),
                    move) != theoreticalMoves.end();
                }),
                possibleMoves.end()
                );

            possibleMoves.erase(
                    std::remove_if(possibleMoves.begin(), possibleMoves.end(),
                        [&](const std::pair<int, int>& move) {
                            return wouldKingBeThreatenedAfterMove(
                                x, y,
                                move.first, move.second,
                                player
                            );
                        }),
                    possibleMoves.end()
                );

        }

    }

    void getPawnAttackMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec) {
        if (player == Players::W_PAWN) {
            if (y > 0) {
                if (x > 0) vec.emplace_back(x - 1, y - 1);
                if (x < 7) vec.emplace_back(x + 1, y - 1);
            }
        }
        else if (player == Players::B_PAWN) {
            if (y < 7) {
                if (x > 0) vec.emplace_back(x - 1, y + 1);
                if (x < 7) vec.emplace_back(x + 1, y + 1);
            }
        }
    }

    void rochade() {
        if (gMap[7][4] == static_cast<int>(Players::EMPTY) || gMap[7][4] != static_cast<int>(Players::W_KING)) {
            whiteKingMoved = true;
        }

        if (gMap[7][7] == static_cast<int>(Players::EMPTY) || gMap[7][7] != static_cast<int>(Players::W_ROOK)) {
            whiteRightRookMoved = true;
        }

        if (gMap[7][0] == static_cast<int>(Players::EMPTY) || gMap[7][0] != static_cast<int>(Players::W_ROOK)) {
            whiteLeftRookMoved = true;
        }

        if (gMap[0][4] == static_cast<int>(Players::EMPTY) || gMap[0][4] != static_cast<int>(Players::B_KING)) {
            blackKingMoved = true;
        }

        if (gMap[0][7] == static_cast<int>(Players::EMPTY) || gMap[0][7] != static_cast<int>(Players::B_ROOK)) {
            blackRightRookMoved = true;
        }

        if (gMap[0][0] == static_cast<int>(Players::EMPTY) || gMap[0][0] != static_cast<int>(Players::B_ROOK)) {
            blackLeftRookMoved = true;
        }
    }

    void getKingAttackMoves(Players p, int x, int y, std::vector<std::pair<int,int>>& moves) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    moves.emplace_back(nx, ny);
                }
            }
        }
    }

    void getPossibleMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec) {

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
                        vec.emplace_back(nx, ny);
                    }

                    //if (player == Players::EMPTY || player >= Players::B_KING) {
                    //    vec.emplace_back(nx, ny);
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


                if (!isThreatend(x, y, false)) {
                    if (!whiteKingMoved && !whiteRightRookMoved && gMap[7][5] == static_cast<int>(Players::EMPTY) && gMap[7][6] == static_cast<int>(Players::EMPTY) && !isThreatend(x+1, y, false) && !isThreatend(x+2, y, false)) {
                        vec.emplace_back(6, 7);
                    }

                    if (!whiteKingMoved && !whiteLeftRookMoved && gMap[7][3] == static_cast<int>(Players::EMPTY) && gMap[7][2] == static_cast<int>(Players::EMPTY) && gMap[7][1] == static_cast<int>(Players::EMPTY) && !isThreatend(x-1, y, false) && !isThreatend(x-2, y, false)) {
                        vec.emplace_back(2, 7);
                    }

                }


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
                        vec.emplace_back(nx, ny);
                    }

                    //if (player == Players::EMPTY || (player >= Players::W_KING && player < Players::B_KING)) {
                    //    vec.emplace_back(nx, ny);
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


                if (!isThreatend(x, y, true)) {
                    if (!blackKingMoved && !blackRightRookMoved && gMap[0][5] == static_cast<int>(Players::EMPTY) && gMap[0][6] == static_cast<int>(Players::EMPTY) && !isThreatend(x+1,y, true) && !isThreatend(x+2,y, true)) {
                        vec.emplace_back(6, 0);
                    }

                    if (!blackKingMoved && !blackLeftRookMoved && gMap[0][3] == static_cast<int>(Players::EMPTY) && gMap[0][2] == static_cast<int>(Players::EMPTY) && gMap[0][1] == static_cast<int>(Players::EMPTY) && !isThreatend(x-1,y, true) && !isThreatend(x-2,y, true)) {
                        vec.emplace_back(2, 0);
                    }
                }



                break;
            }

            case Players::W_QUEEN: {

                auto tryMove = [&](int dx, int dy) {

                    bool hitEnemy = false;

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
                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                            }

                        }
                        else if (isWhitePiece(player)) {
                            break;
                        }
                        else if (isBlackPiece(player)) {
                            // Fuegt das Feld hinter der gegnerischen Figur nicht mehr hinzu
                            // vec.emplace_back(nx, ny);
                            // break;


                            // Wenn hier gelandet: Flag setzen
                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                                hitEnemy = true;
                            }

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

                    bool hitEnemy = false;

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
                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                            }
                        }
                        else if (isBlackPiece(player)) {
                            break;
                        }
                        else if (isWhitePiece(player)) {
                            // Fuegt das Feld hinter der gegnerischen Figur nicht mehr hinzu
                            // vec.emplace_back(nx, ny);
                            // break;


                            // Wenn hier gelandet: Flag setzen
                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                                hitEnemy = true;
                            }

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

                bool hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isBlackPiece(player)) {
                        if (player == Players::B_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;

                hitEnemy = false;

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
                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }
                    if (isBlackPiece(player)) {

                        if (player == Players::B_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }

                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;

                hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isBlackPiece(player)) {
                        if (player == Players::B_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;

                hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isBlackPiece(player)) {
                        if (player == Players::B_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }



                break;
            }
            case Players::B_ROOK: {
                std::cout << "B Rook" << std::endl;

                // While-Schleife um zu ueberpruefen, wie viele Felder frei sind
                // Richtung nach oben
                int nx = x, ny = y;

                bool hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isWhitePiece(player)) {
                        if (player == Players::W_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }

                // Richtung nach unten
                nx = x;
                ny = y;

                hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isWhitePiece(player)) {
                        if (player == Players::W_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }

                // Richtung nach links
                nx = x;
                ny = y;

                hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isWhitePiece(player)) {
                        if (player == Players::W_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }
                    }
                }

                // Richtung nach rechts
                nx = x;
                ny = y;

                hitEnemy = false;

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

                    if (hitEnemy) {
                        theoreticalMoves.emplace_back(nx, ny);
                    }
                    else {
                        vec.emplace_back(nx, ny);
                    }

                    if (isWhitePiece(player)) {

                        if (player == Players::W_KING) {
                            hitEnemy = true;
                        }
                        else {
                            break;
                        }

                    }

                }

                break;
            }

            case Players::W_BISHOP: {

                int startX = x;
                int startY = y;

                auto tryMove = [&](int dx, int dy) {

                    bool hitEnemy = false;

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

                            if (player == Players::B_KING) {
                                vec.emplace_back(nx, ny);
                                hitEnemy = true;
                            }
                            else {
                                vec.emplace_back(nx, ny);
                                break;
                            }

                        }
                        // Leeres Feld ist begehbar
                        if (isEmpty(player)) {

                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                            }

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

                    bool hitEnemy = false;

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
                            if (player == Players::W_KING) {
                                vec.emplace_back(nx, ny);
                                hitEnemy = true;
                            }
                            else {
                                vec.emplace_back(nx, ny);
                                break;
                            }
                        }
                        // Leeres Feld ist begehbar
                        if (isEmpty(player)) {
                            if (hitEnemy) {
                                theoreticalMoves.emplace_back(nx, ny);
                            }
                            else {
                                vec.emplace_back(nx, ny);
                            }
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
                            vec.emplace_back(tx, ty);
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
                            vec.emplace_back(tx, ty);
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
                        vec.emplace_back(x, y-1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 6 && isEmpty(static_cast<Players>(gMap[y-2][x]))) {
                            vec.emplace_back(x, y-2);
                        }
                    }
                    // Wenn das Feld links oben links nicht leer ist
                    if (x > 0 && !isEmpty(static_cast<Players>(gMap[y-1][x-1]))) {
                        // Wenn auf dem Feld eine weisse Schachfigur ist
                        if (isWhitePiece(static_cast<Players>(gMap[y-1][x-1]))){

                        }
                        // Wenn auf dem Feld eine schwarze Schachfigur ist
                        else {
                            vec.emplace_back(x-1, y-1);

                        }
                    }
                    // Wenn das Feld oben rechts nicht leer ist
                    if (x < 7 && !isEmpty(static_cast<Players>(gMap[y-1][x+1]))) {
                        // Wenn auf dem Feld eine weisse Schachfigur ist
                        if (isWhitePiece(static_cast<Players>(gMap[y-1][x+1]))){
                        }
                        // Wenn auf dem Feld eine schwarze Schachfigur ist
                        else {
                            vec.emplace_back(x+1, y-1);

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
                        vec.emplace_back(x, y+1);
                        // y == 6 => Weisser Bauer ist noch auf seiner Startlinie
                        if (y == 1 && isEmpty(static_cast<Players>(gMap[y+2][x]))) {
                            vec.emplace_back(x, y+2);
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
                            vec.emplace_back(x-1, y+1);
                        }
                    }
                    if (x < 7 && !isEmpty(static_cast<Players>(gMap[y+1][x+1]))) {
                        if (isBlackPiece(static_cast<Players>(gMap[y+1][x+1]))){
                        }
                        else {
                            vec.emplace_back(x+1, y+1);
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

bool Game::whiteKingMoved = false;
bool Game::blackKingMoved = false;
bool Game::whiteRightRookMoved = false;
bool Game::blackRightRookMoved = false;
bool Game::whiteLeftRookMoved = false;
bool Game::blackLeftRookMoved = false;


int main() {

    const int screenW = 1400;
    const int screenH = 1200;

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

            game.loop();

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}