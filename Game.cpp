#include "Game.h"

#include <iostream>
#include "raylib.h"
#include <vector>
#include <utility>
#include <algorithm>
#include <complex>
#include <format>
#include <list>
#include <tuple>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <array>
#include <stack>
#include "tinyfiledialogs.h"


    Game::Game(int fieldlength) : fieldlength{fieldlength}, blocksize{fieldlength / 8} {
        this->midX = GetScreenWidth() / 2;
        this->midY = GetScreenHeight() / 2;

        fieldX = midX - (fieldlength / 2);
        fieldY = midY - (fieldlength / 2);

        field = Rectangle{static_cast<float>(fieldX), static_cast<float>(fieldY), static_cast<float>(fieldlength), static_cast<float>(fieldlength)};

        restartW = fieldlength/2;
        restartH = fieldlength / 4;
        restartX = this->fieldX + fieldlength/4;
        restartY = GetScreenHeight() - restartH - 10;


        saveLogButton.width = fieldlength/2;
        saveLogButton.height = 150;
        saveLogButton.x = GetScreenWidth() - saveLogButton.width - 10;
        saveLogButton.y = GetScreenHeight() - saveLogButton.height - 10;

        loadLogButton.width = saveLogButton.width;
        loadLogButton.height = saveLogButton.height;
        loadLogButton.x = 10;
        loadLogButton.y = saveLogButton.y;


        goBackButton.width = blocksize;
        goBackButton.height = blocksize;
        goBackButton.x = blocksize / 5;
        goBackButton.y = blocksize * 2;

        goForwardButton.width = goBackButton.width;
        goForwardButton.height = goBackButton.height;
        goForwardButton.x = goBackButton.x + goBackButton.width + 10;
        goForwardButton.y = goBackButton.y;

        fields.push_back(currentState());
        fieldsIndex = 0;


        pieceSpritesheet = LoadTexture("../images/chesspieces.png");

        // Ganzes Sheet in 12 Bloecke aufteilen
        auto tileW = static_cast<float>(pieceSpritesheet.width / 6.0);
        auto tileH = static_cast<float>(pieceSpritesheet.height / 2.0);
        for (int i = 0; i < 6; i++) {
            pieceSprites[i] = Rectangle{static_cast<float>(i)*tileW, 0, tileW, tileH};
        }
        for (int i = 0; i < 6; i++) {
            pieceSprites[6 + i] = Rectangle{static_cast<float>(i)*tileW, tileH, tileW, tileH};
        }

        updateKingPosition();

        logRectangle = Rectangle(static_cast<float>(this->fieldX), 10, static_cast<float>(this->fieldlength), 5 * fieldlength / 20 + 20);

    }

    Game::~Game() {
        UnloadTexture(pieceSpritesheet);
    }

    bool Game::mouseCollision(Vector2 mousePos, Rectangle rec){

        if (mousePos.x >= rec.x && mousePos.x <= (rec.x + rec.width) &&
            mousePos.y >= rec.y && mousePos.y <= (rec.y + rec.height)) \
        {
            return true;
        }
        return false;
    }

    void Game::loop() {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            // Maus hat auf Restart geklickt
            if (mousePos.x >= static_cast<float>(restartX) && mousePos.x <= static_cast<float>(restartX + restartW)
                && mousePos.y >= static_cast<float>(restartY) && mousePos.y <= static_cast<float>(restartY + restartH)) {

                restart();

                }


            // Maus hat auf SaveLog geklickt
            if (mousePos.x >= static_cast<float>(saveLogButton.x) && mousePos.x <= static_cast<float>(saveLogButton.x + saveLogButton.width)
                && mousePos.y >= static_cast<float>(saveLogButton.y) && mousePos.y <= static_cast<float>(saveLogButton.y + saveLogButton.height)) {

                saveLogs();

                }


            if (mousePos.x >= static_cast<float>(loadLogButton.x) && mousePos.x <= static_cast<float>(loadLogButton.x + loadLogButton.width)
                && mousePos.y >= static_cast<float>(loadLogButton.y) && mousePos.y <= static_cast<float>(loadLogButton.y + loadLogButton.height)) {

                std::cout << "Load Log" << std::endl;

                loadLog();

                }

            if (mousePos.x >= static_cast<float>(goBackButton.x) && mousePos.x <= static_cast<float>(goBackButton.x + goBackButton.width)
                && mousePos.y >= static_cast<float>(goBackButton.y) && mousePos.y <= static_cast<float>(goBackButton.y + goBackButton.height)) {

                std::cout << "Go Back" << std::endl;

                if (fieldsIndex != 0 && !logList.empty()) {
                    fieldsIndex--;
                    loadState(fields[fieldsIndex]);

                    won = Won::NONE;
                    check = Check::CHECK_NONE;
                    gameMode = GameMode::PLAY;

                    bool whiteInCheck = isThreatend(whiteKingPosition.first, whiteKingPosition.second, false);
                    bool blackInCheck = isThreatend(blackKingPosition.first, blackKingPosition.second, true);

                    if (whiteInCheck) {
                        check = Check::CHECK_BLACK;
                    }
                    else if (blackInCheck) {
                        check = Check::CHECK_WHITE;
                    }

                    auto element = logList.back();

                    bool hit = std::get<2>(element.second);
                    Players playerWhoMoved = element.first;

                    if (hit) {
                        // Weiß hatte geschlagen -> schwarze Figur wurde verloren
                        if (isWhitePiece(playerWhoMoved) && !lostBlackPieces.empty()) {
                            recoveryLostBlackPieces.push(lostBlackPieces.back());
                            lostBlackPieces.pop_back();
                        }

                        // Schwarz hatte geschlagen -> weiße Figur wurde verloren
                        else if (isBlackPiece(playerWhoMoved) && !lostWhitePieces.empty()) {
                            recoveryLostWhitePieces.push(lostWhitePieces.back());
                            lostWhitePieces.pop_back();
                        }
                    }


                    recoveryLogList.push(element);
                    logList.pop_back();

                    rebuildActualLogs();

                    turn = !turn;
                    clickedField = {-1, -1};
                    possibleMoves.clear();
                }


                }


            if (mousePos.x >= static_cast<float>(goForwardButton.x) && mousePos.x <= static_cast<float>(goForwardButton.x + goForwardButton.width)
                && mousePos.y >= static_cast<float>(goForwardButton.y) && mousePos.y <= static_cast<float>(goForwardButton.y + goForwardButton.height)) {

                std::cout << "Go Forward" << std::endl;

                if (fieldsIndex + 1 < fields.size() && !recoveryLogList.empty()) {
                    fieldsIndex++;
                    loadState(fields[fieldsIndex]);

                    won = Won::NONE;
                    check = Check::CHECK_NONE;
                    gameMode = GameMode::PLAY;

                    bool whiteInCheck = isThreatend(whiteKingPosition.first, whiteKingPosition.second, false);
                    bool blackInCheck = isThreatend(blackKingPosition.first, blackKingPosition.second, true);

                    if (whiteInCheck) {
                        check = Check::CHECK_BLACK;
                    }
                    else if (blackInCheck) {
                        check = Check::CHECK_WHITE;
                    }

                    auto element = recoveryLogList.top();

                    bool hit = std::get<2>(element.second);
                    Players playerWhoMoved = element.first;

                    if (hit) {
                        if (isWhitePiece(playerWhoMoved) && !recoveryLostBlackPieces.empty()) {
                            lostBlackPieces.push_back(recoveryLostBlackPieces.top());
                            recoveryLostBlackPieces.pop();
                        }
                        else if (isBlackPiece(playerWhoMoved) && !recoveryLostWhitePieces.empty()) {
                            lostWhitePieces.push_back(recoveryLostWhitePieces.top());
                            recoveryLostWhitePieces.pop();
                        }
                    }


                    recoveryLogList.pop();
                    logList.push_back(element);

                    rebuildActualLogs();

                    turn = !turn;
                    clickedField = {-1, -1};
                    possibleMoves.clear();
                }


                }

        }

        if (gameMode == GameMode::PLAY) {
            update();
        }
        else if (gameMode == GameMode::CHOOSE_MODE){
            // Wenn die Maus eine Figur ausgewaehlt hat
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                //Hier irgendwie dann die Figuren aus der Liste auswaehlen koennen
                //Auflistung ist so:

                // Die fontSize 30 ist die hoehe von den Feldern
                int fontSize = this->chooseRecH;
                int xPos = this->fieldX + 10;
                int yPos = this->fieldY + 10 + 40 + fontSize;

                for (int i = 0; i < allWhitePieces.size(); i++) {
                    Rectangle rec(static_cast<float>(xPos), static_cast<float>(yPos), static_cast<float>(this->chooseRecW), static_cast<float>(this->chooseRecH));
                    if (mouseCollision(mousePos, rec)) {

                        switch (i) {
                            case 0:

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_QUEEN);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_QUEEN);
                                }
                                break;
                            case 1:

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_ROOK);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_ROOK);
                                }
                                break;
                            case 2:

                                if (!turn) {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::W_BISHOP);
                                }
                                else {
                                    gMap[lastMove.second][lastMove.first] = static_cast<int>(Players::B_BISHOP);
                                }

                                break;
                            case 3:

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

    void Game::loadLog() {

        Board startMap{{
            {9 ,11,10,8 ,7 ,10,11,9},
            {12,12,12,12,12,12,12,12},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {6 ,6 ,6 ,6 ,6 ,6 ,6 ,6},
            {3 ,5 ,4 ,2 ,1 ,4 ,5 ,3},
        }};

        gMap = startMap;
        turn = true;

        whiteKingMoved = false;
        blackKingMoved = false;
        whiteRightRookMoved = false;
        blackRightRookMoved = false;
        whiteLeftRookMoved = false;
        blackLeftRookMoved = false;

        lostBlackPieces.clear();
        lostWhitePieces.clear();
        recoveryLostBlackPieces = {};
        recoveryLostWhitePieces = {};

        logList.clear();
        actualLogs.clear();
        recoveryLogList = {};

        fields.clear();
        fields.push_back(currentState());
        fieldsIndex = 0;

        clickedField = {-1, -1};
        possibleMoves.clear();

        check = Check::CHECK_NONE;
        won = Won::NONE;
        gameMode = GameMode::PLAY;

        updateKingPosition();

        const char* filters[] = {"*.txt"};

        const char* filePath = tinyfd_openFileDialog(
            "Logdatei auswaehlen",
            "./Logs/",
            1,
            filters,
            "Textdateien",
            0
        );

        if (filePath == nullptr) {
            std::cout << "Keine Datei ausgewaehlt" << std::endl;
            return;
        }

        std::filesystem::path data = filePath;
        std::ifstream logFile(data);

        std::string line;

        while (std::getline(logFile, line)) {
            std::istringstream iss(line);

            std::string player;
            std::string ignore;
            std::string location;

            iss >> player >> ignore >> location;

            Players figure = Players::EMPTY;

            if (player == "W_KING") figure = Players::W_KING;
            else if (player == "W_QUEEN") figure = Players::W_QUEEN;
            else if (player == "W_ROOK") figure = Players::W_ROOK;
            else if (player == "W_BISHOP") figure = Players::W_BISHOP;
            else if (player == "W_KNIGHT") figure = Players::W_KNIGHT;
            else if (player == "W_PAWN") figure = Players::W_PAWN;
            else if (player == "B_KING") figure = Players::B_KING;
            else if (player == "B_QUEEN") figure = Players::B_QUEEN;
            else if (player == "B_ROOK") figure = Players::B_ROOK;
            else if (player == "B_BISHOP") figure = Players::B_BISHOP;
            else if (player == "B_KNIGHT") figure = Players::B_KNIGHT;
            else if (player == "B_PAWN") figure = Players::B_PAWN;
            else {
                std::cout << "Unknown player: " << player << std::endl;
                continue;
            }

            if (location == "0-0" || location == "O-O") {
                int y = isWhitePiece(figure) ? 7 : 0;

                if (figure == Players::W_KING) {
                    gMap[7][6] = static_cast<int>(Players::W_KING);
                    gMap[7][5] = static_cast<int>(Players::W_ROOK);
                    gMap[7][4] = static_cast<int>(Players::EMPTY);
                    gMap[7][7] = static_cast<int>(Players::EMPTY);

                    whiteKingMoved = true;
                    whiteRightRookMoved = true;
                }
                else if (figure == Players::B_KING) {
                    gMap[0][6] = static_cast<int>(Players::B_KING);
                    gMap[0][5] = static_cast<int>(Players::B_ROOK);
                    gMap[0][4] = static_cast<int>(Players::EMPTY);
                    gMap[0][7] = static_cast<int>(Players::EMPTY);

                    blackKingMoved = true;
                    blackRightRookMoved = true;
                }

                logList.emplace_back(figure, std::make_tuple(6, y, false));
                rebuildActualLogs();

                updateKingPosition();

                fields.push_back(currentState());
                fieldsIndex = fields.size() - 1;

                turn = !turn;
                continue;
            }

            if (location == "0-0-0" || location == "O-O-O") {
                int y = isWhitePiece(figure) ? 7 : 0;

                if (figure == Players::W_KING) {
                    gMap[7][2] = static_cast<int>(Players::W_KING);
                    gMap[7][3] = static_cast<int>(Players::W_ROOK);
                    gMap[7][4] = static_cast<int>(Players::EMPTY);
                    gMap[7][0] = static_cast<int>(Players::EMPTY);

                    whiteKingMoved = true;
                    whiteLeftRookMoved = true;
                }
                else if (figure == Players::B_KING) {
                    gMap[0][2] = static_cast<int>(Players::B_KING);
                    gMap[0][3] = static_cast<int>(Players::B_ROOK);
                    gMap[0][4] = static_cast<int>(Players::EMPTY);
                    gMap[0][0] = static_cast<int>(Players::EMPTY);

                    blackKingMoved = true;
                    blackLeftRookMoved = true;
                }

                logList.emplace_back(figure, std::make_tuple(2, y, false));
                rebuildActualLogs();

                updateKingPosition();

                fields.push_back(currentState());
                fieldsIndex = fields.size() - 1;

                turn = !turn;
                continue;
            }

            bool hit = false;

            if (!location.empty() && location[0] == 'x') {
                hit = true;
                location.erase(0, 1);
            }

            if (location.size() < 2) {
                std::cout << "Invalid location: " << location << std::endl;
                continue;
            }

            char cx = location[0];
            int rank = std::stoi(std::string(1, location[1]));

            int x = -1;
            int y = 8 - rank;

            switch (cx) {
                case 'A': x = 0; break;
                case 'B': x = 1; break;
                case 'C': x = 2; break;
                case 'D': x = 3; break;
                case 'E': x = 4; break;
                case 'F': x = 5; break;
                case 'G': x = 6; break;
                case 'H': x = 7; break;
                default:
                    std::cout << "Invalid file: " << cx << std::endl;
                    continue;
            }

            std::pair<int, int> target = {x, y};

            std::vector<std::pair<int, int>> moves;
            bool moveLoaded = false;

            for (int iy = 0; iy < static_cast<int>(gMap.size()) && !moveLoaded; iy++) {
                for (int jx = 0; jx < static_cast<int>(gMap[iy].size()) && !moveLoaded; jx++) {
                    if (gMap[iy][jx] == static_cast<int>(figure)) {
                        moves.clear();
                        getPossibleMoves(figure, jx, iy, moves);

                        auto it = std::ranges::find(moves, target);

                        if (it != moves.end()) {
                            Players captured = static_cast<Players>(gMap[target.second][target.first]);
                            bool realHit = !isEmpty(captured);

                            if (realHit) {
                                if (isWhitePiece(figure)) {
                                    lostBlackPieces.push_back(captured);
                                }
                                else if (isBlackPiece(figure)) {
                                    lostWhitePieces.push_back(captured);
                                }
                            }

                            if (figure == Players::W_KING) whiteKingMoved = true;
                            else if (figure == Players::B_KING) blackKingMoved = true;
                            else if (figure == Players::W_ROOK) {
                                if (jx == 0 && iy == 7) whiteLeftRookMoved = true;
                                else if (jx == 7 && iy == 7) whiteRightRookMoved = true;
                            }
                            else if (figure == Players::B_ROOK) {
                                if (jx == 0 && iy == 0) blackLeftRookMoved = true;
                                else if (jx == 7 && iy == 0) blackRightRookMoved = true;
                            }

                            gMap[target.second][target.first] = gMap[iy][jx];
                            gMap[iy][jx] = static_cast<int>(Players::EMPTY);

                            logList.emplace_back(figure, std::make_tuple(target.first, target.second, realHit));
                            rebuildActualLogs();

                            updateKingPosition();

                            fields.push_back(currentState());
                            fieldsIndex = fields.size() - 1;

                            turn = !turn;
                            moveLoaded = true;
                        }
                    }
                }
            }

            if (!moveLoaded) {
                std::cout << "Could not load move: " << line << std::endl;
            }
        }
    }

    // Spiel zeichnen
    void Game::draw() {
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
            //saveLogs();

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
            if (this->won == Won::DRAW) {
                DrawText("Patt!", this->fieldX, this->fieldY - 100, 100, BLACK);
                gameMode = GameMode::END;
            }
        }

        drawRestartButton();

        drawSaveLogButton();

        drawLoadLogButton();

        drawGoBackButton();

        drawGoForwardButton();

    }

    void Game::drawLoadLogButton() {

        DrawRectangleRec(loadLogButton, WHITE);
        DrawRectangleLines(loadLogButton.x + 10, loadLogButton.y + 10, loadLogButton.width - 20, loadLogButton.height - 20, BLACK);
        int fontSize = fieldlength / 12;
        int width = MeasureText("Load Log", fontSize);
        DrawText("Load Log", loadLogButton.x + loadLogButton.width/2 - width/2, this->loadLogButton.y + loadLogButton.height/2 - fontSize/2, fontSize, BLACK);

    }

    void Game::drawGoForwardButton() {

        DrawRectangleRec(goForwardButton, WHITE);
        DrawRectangleLines(goForwardButton.x + 10, goForwardButton.y + 10, goForwardButton.width - 20, goForwardButton.height - 20, BLACK);
        int fontSize = fieldlength / 30;
        int width = MeasureText("->", fontSize);
        DrawText("->", goForwardButton.x + goForwardButton.width/2 - width/2, this->goForwardButton.y + goForwardButton.height/2 - fontSize/2, fontSize, BLACK);

    }

    void Game::drawGoBackButton() {

        DrawRectangleRec(goBackButton, WHITE);
        DrawRectangleLines(goBackButton.x + 10, goBackButton.y + 10, goBackButton.width - 20, goBackButton.height - 20, BLACK);
        int fontSize = fieldlength / 30;
        int width = MeasureText("<-", fontSize);
        DrawText("<-", goBackButton.x + goBackButton.width/2 - width/2, this->goBackButton.y + goBackButton.height/2 - fontSize/2, fontSize, BLACK);

    }

    void Game::drawSaveLogButton() {
        DrawRectangleRec(saveLogButton, WHITE);
        DrawRectangleLines(saveLogButton.x + 10, saveLogButton.y + 10, saveLogButton.width - 20, saveLogButton.height - 20, BLACK);
        int fontSize = fieldlength / 12;
        int width = MeasureText("Save Log", fontSize);
        DrawText("Save Log", saveLogButton.x + saveLogButton.width/2 - width/2, this->saveLogButton.y + saveLogButton.height/2 - fontSize/2, fontSize, BLACK);
    }


    void Game::drawRestartButton() {
        DrawRectangle(restartX, restartY, restartW, restartH, WHITE);
        DrawRectangleLines(restartX+10, restartY + 10, restartW - 20, restartH - 20, BLACK);
        int fontSize = fieldlength / 12;
        int width = MeasureText("Restart!", fontSize);
        DrawText("Restart!", this->midX - width/2, this->restartY + restartH/2 - fontSize/2, fontSize, BLACK);
    }

    bool Game::isBoardFlipped() const {
        return !turn; // Schwarz am Zug => Schwarz unten
    }

    std::pair<int, int> Game::boardToView(int x, int y) const {
        if (!isBoardFlipped()) return {x, y};
        return {7 - x, 7 - y};
    }

    std::pair<int, int> Game::viewToBoard(int x, int y) const {
        if (!isBoardFlipped()) return {x, y};
        return {7 - x, 7 - y};
    }


    // Regeln
    void Game::update() {
        moveFigure();
    }

    std::string Game::playerToString(const Players& player) {
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
            case Players::EMPTY:
                return "";
                break;
        }

        return "";
    }

    std::string Game::coordinatesToLabels(int x, int y) {


        std::string text;
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
            default:
                text += "ERROR";
                break;
        }

        std::vector<int> tmp{8,7,6,5,4,3,2,1};

        text += std::to_string(tmp.at(y));


        return text;

    }


    void Game::saveLogs() {

        // logs_UHRZEIT.txt;

        /*
        if (created) {
            return;
        }
        */

        if (logList.empty()) return;

        std::stringstream ss;

        for (const auto& var : logList) {

            Players player = var.first;
            std::string text = playerToString(player);

            int x = std::get<0>(var.second);
            int y = std::get<1>(var.second);
            bool hit = std::get<2>(var.second);

            if (x == -1 && y == -1) {
                text += " -> 0-0";
            }
            else if (x == -2 && y == -2) {
                text += " -> 0-0-0";
            }
            else {
                std::string location = coordinatesToLabels(x, y);
                text += std::string(" -> ") + (hit ? "x" : "") + location;
            }

            ss << text << "\n";

        }

        std::filesystem::path folder = "Logs";

        if (!std::filesystem::exists(folder)) {
            std::filesystem::create_directory(folder);
        }

        auto now = std::chrono::system_clock::now();
        auto now_sec = std::chrono::floor<std::chrono::seconds>(now);

        std::string date = std::format("{:%Y-%m-%d_%H-%M-%S}", now_sec);

        std::string wonText;
        if (won == Won::WON_WHITE) {
            wonText = "WHITE_WON";
        }
        else if (won == Won::WON_BLACK) {
            wonText = "BLACK_WON";
        }
        else if (won == Won::DRAW) {
            wonText = "DRAW";
        }
        else {
            wonText = "INGAME";
        }

        std::string filename = "logs_" + date + "_" + wonText + ".txt";

        std::ofstream file(folder / filename);

        if (file.is_open()) {
            file << ss.str();
            file.close();
        }
        else {
            std::cout << "Could not open file" << std::endl;
        }


        created = true;

    }



    void Game::scrollLogList() {

        // 0 = Kein Scrollen
        // >0 = nach oben Scrollen
        // <0 = nach unten Scrollen
        float wheel = GetMouseWheelMove();

        if (actualLogs.empty()) return;
        if (logList.empty()) return;


        if (wheel > 0 && actualLogs.size() == 10 && actualLogs.front() > 0) {
            actualLogs.pop_back();
            actualLogs.push_front(actualLogs.front() - 1);
        }

        if (wheel < 0 && actualLogs.size() == 10 && actualLogs.back() < static_cast<int>(logList.size()) - 1) {
            actualLogs.pop_front();
            actualLogs.push_back(actualLogs.back() + 1);
        }

    }


    void Game::drawLogList() {

        DrawRectangleRec(logRectangle, {153,152,92,150});

        DrawText("Scroll with the mouse", logRectangle.x + logRectangle.width + 20, logRectangle.y + 10, fieldlength / 25, BLACK);
        DrawText("to see the moves", logRectangle.x + logRectangle.width + 20, logRectangle.y + 40, fieldlength / 25, BLACK);


        DrawLine(static_cast<int>(logRectangle.x + logRectangle.width/2),
            static_cast<int>(logRectangle.y + 10),
            static_cast<int>(logRectangle.x + logRectangle.width/2),
            static_cast<int>(logRectangle.y + logRectangle.height - 10),
            {110, 95, 0, 255});

        //DrawText("test", logRectangle.x + 10, logRectangle.y + 10, 30, BLACK);

        // Eintraege der Liste printen - Weiss links - Schwarz rechts
        int wCount = 0;
        int bCount = 0;

        scrollLogList();


        for (const int& index : actualLogs) {

            if (index < 0 || index >= logList.size()) continue;


            auto var = logList.at(index);


            Players player = var.first;
            std::string text = playerToString(player);

            int x = std::get<0>(var.second);
            int y = std::get<1>(var.second);
            bool hit = std::get<2>(var.second);


            if (x == -1 && y == -1) {
                text += " -> 0-0";
            }
            else if (x == -2 && y == -2) {
                text += " -> 0-0-0";
            }
            else {
                std::string location = coordinatesToLabels(x, y);
                text += std::string(" -> ") + (hit ? "x" : "") + location;
            }

            int fontSize = fieldlength / 20;

            // Weiss auf der linken Spalte
            if (isWhitePiece(player)) {
                DrawText(text.c_str(), static_cast<int>(logRectangle.x + 10), static_cast<int>(logRectangle.y + 10 + static_cast<float>(fontSize) * static_cast<float>(wCount)), fontSize, WHITE);
                wCount++;
            }
            // Schwarz auf der rechten Spalte
            else if (isBlackPiece(player)) {
                DrawText(text.c_str(), static_cast<int>(logRectangle.x + logRectangle.width / 2 + 10), static_cast<int>(logRectangle.y + 10 + static_cast<float>(fontSize) * static_cast<float>(bCount)), fontSize, BLACK);
                bCount++;
            }


        }

    }


    void Game::drawEndField() const {
        DrawRectangleRec(field, {200, 96, 232,200});

        if (won == Won::WON_WHITE) {
            std::string text = "Weiss hat gewonnen!";

            int fontSize = 30;
            int textWidth = MeasureText(text.c_str(), fontSize);

            DrawText(text.c_str(), static_cast<int>(field.x + (field.width / 2) - static_cast<float>(textWidth)/2), static_cast<int>(field.y + (field.height / 2) - static_cast<float>(fontSize)/2), fontSize, WHITE);
        }
        else if (won == Won::WON_BLACK) {
            std::string text = "Schwarz hat gewonnen!";

            int fontSize = 30;
            int textWidth = MeasureText(text.c_str(), fontSize);

            DrawText(text.c_str(), static_cast<int>(field.x + (field.width / 2) - static_cast<float>(textWidth)/2), static_cast<int>(field.y + (field.height / 2) - static_cast<float>(fontSize)/2), fontSize, BLACK);
        }


    }


    void Game::drawChooseField() {

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

    void Game::drawFiguresLost(std::vector<Players>& vect, int blackXoff, int whiteXoff, int fontSize, int offset, int yOff, bool background, Color backgroundColor) {
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

    // Wenn nur noch ein Spieler auf dem Feld ist und dieser keinen PossibleMove hat, dann Schachmatt!

    void Game::drawLostFigures() {


        // Schwarze Figuren
        int fontSize = fieldlength / 20;
        DrawText("Verluste von Schwarz:", 10, this->fieldY - fontSize, fontSize, BLACK);

        int blackXoff = 10;

        int offset = 1;

        DrawRectangleRec(Rectangle(blackXoff - 5, this->fieldY + fontSize - 10, fieldlength / 2 - 50, 20 * fontSize + 20), {168, 35, 25, 200});

        std::vector<Players> black(lostBlackPieces.begin(), lostBlackPieces.end());

        drawFiguresLost(black, blackXoff, 0, fontSize, offset,  this->fieldY);



        int whiteXoff = GetScreenWidth() - (fieldlength / 2 - 50) - 5;

        int textWidth = MeasureText("Verluste von Weiss:", fontSize);

        DrawText("Verluste von Weiss:", GetScreenWidth() - textWidth - 10, this->fieldY - fontSize, fontSize, WHITE);

        DrawRectangleRec(Rectangle(whiteXoff, this->fieldY + fontSize - 10, fieldlength / 2 - 50, 20 * fontSize + 20), {168, 35, 25, 200});


        std::vector<Players> white(lostWhitePieces.begin(), lostWhitePieces.end());

        drawFiguresLost(white, 0, whiteXoff + 10, fontSize, offset,  this->fieldY);


    }



    void Game::drawText() const {
        DrawText(TextFormat("%s to move", (this->turn ? "White" : "Black"), 20), 10, 10, fieldlength / 15, (this->turn ? WHITE : BLACK));
    }

    // Spielfeld zeichnen
    void Game::drawField() const {
        DrawRectangleRec(field, {206,130,64,255});

        for (int viewY = 0; viewY < 8; viewY++) {
            for (int viewX = 0; viewX < 8; viewX++) {

                auto [boardX, boardY] = viewToBoard(viewX, viewY);

                int nx = fieldX + blocksize * viewX;
                int ny = fieldY + blocksize * viewY;

                if ((boardX + boardY) % 2 == 0) {
                    DrawRectangle(nx, ny, blocksize, blocksize, {119,119,119,255});
                }
            }
        }

        drawLabels();
    }

    void Game::drawLabels() const {

        int fontsize = 30;
        int offset = 20;

        for (int viewX = 0; viewX < 8; viewX++) {
            int boardX = isBoardFlipped() ? 7 - viewX : viewX;
            char label = 'A' + boardX;

            int textWidth = MeasureText(TextFormat("%c", label), fontsize);

            DrawText(TextFormat("%c", label),
                fieldX + blocksize/2 - textWidth/2 + viewX * blocksize,
                fieldY + fieldlength + offset,
                fontsize, BLACK);
        }

        for (int viewY = 0; viewY < 8; viewY++) {
            int boardY = isBoardFlipped() ? 7 - viewY : viewY;
            int number = 8 - boardY;

            int textWidth = MeasureText(TextFormat("%d", number), fontsize);

            DrawText(TextFormat("%d", number),
                fieldX - textWidth - offset,
                fieldY + blocksize/2 + viewY * blocksize - fontsize/2,
                fontsize, BLACK);
        }
    }

    void Game::drawClickedField() {
        if (clickedField.first != -1 && clickedField.second != -1) {

            auto [vx, vy] = boardToView(clickedField.first, clickedField.second);

            int nx = fieldX + (blocksize * vx);
            int ny = fieldY + (blocksize * vy);

            DrawRectangle(nx, ny, blocksize, blocksize, {255,0,0,255});
            DrawRectangleLines(nx, ny, blocksize, blocksize, BLACK);

            drawPossibleMoves();
        }
    }



    void Game::drawFigures() {

        float scale = static_cast<float>(fieldlength) / 240;

        auto drawFiguresHelpMethod = [&](PieceSprite pieceSprite, int nx, int ny) {
            Rectangle src = pieceSprites[static_cast<int>(pieceSprite)];
            auto dst = Rectangle{static_cast<float>(nx), static_cast<float>(ny), src.width * scale, src.height * scale};
            DrawTexturePro(pieceSpritesheet, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        };

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {

                auto [vx, vy] = boardToView(x, y);

                int nx = fieldX + (blocksize * vx) + (blocksize / 6);
                int ny = fieldY + (blocksize * vy);

                switch (static_cast<Players>(gMap[y][x])) {
                    case Players::EMPTY: break;

                    case Players::W_KING: drawFiguresHelpMethod(PieceSprite::W_KING, nx, ny); break;
                    case Players::W_QUEEN: drawFiguresHelpMethod(PieceSprite::W_QUEEN, nx, ny); break;
                    case Players::W_ROOK: drawFiguresHelpMethod(PieceSprite::W_ROOK, nx, ny); break;
                    case Players::W_BISHOP: drawFiguresHelpMethod(PieceSprite::W_BISHOP, nx, ny); break;
                    case Players::W_KNIGHT: drawFiguresHelpMethod(PieceSprite::W_KNIGHT, nx, ny); break;
                    case Players::W_PAWN: drawFiguresHelpMethod(PieceSprite::W_PAWN, nx, ny); break;

                    case Players::B_KING: drawFiguresHelpMethod(PieceSprite::B_KING, nx, ny); break;
                    case Players::B_QUEEN: drawFiguresHelpMethod(PieceSprite::B_QUEEN, nx, ny); break;
                    case Players::B_ROOK: drawFiguresHelpMethod(PieceSprite::B_ROOK, nx, ny); break;
                    case Players::B_BISHOP: drawFiguresHelpMethod(PieceSprite::B_BISHOP, nx, ny); break;
                    case Players::B_KNIGHT: drawFiguresHelpMethod(PieceSprite::B_KNIGHT, nx, ny); break;
                    case Players::B_PAWN: drawFiguresHelpMethod(PieceSprite::B_PAWN, nx, ny); break;

                    default: break;
                }
            }
        }
    }



    // Returns False when player is an empty grid
    bool Game::isEnemyPiece(const Players& player) {

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

    bool Game::isEmpty(const Players& player) {
        return player == Players::EMPTY;
    }

    // Returns True when player is a black piece
    bool Game::isBlackPiece(const Players& player){
        return player >= Players::B_KING;
    }

    bool Game::isWhitePiece(const Players& player) {
        return player >= Players::W_KING && player < Players::B_KING;
    }


    void Game::restart() {

         Board startMap{{
            {9 ,11,10,8 ,7 ,10,11,9},
            {12 ,12 ,12 ,12 ,12 ,12 ,12 ,12},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
            {6 ,6 ,6 ,6 ,6 ,6 ,6 ,6},
            {3 ,5 ,4 ,2 ,1 ,4 ,5 ,3},
        }};

        gMap = startMap;
        // Weiss ist am Zug
        turn = true;

        // Reset
        lostBlackPieces.clear();
        lostWhitePieces.clear();

        logList.clear();
        actualLogs.clear();
        recoveryLogList = std::stack<std::pair<Players, std::tuple<int,int, bool>>>();
        recoveryLostBlackPieces = std::stack<Players>();
        recoveryLostWhitePieces = std::stack<Players>();


        check = Check::CHECK_NONE;
        won = Won::NONE;
        gameMode = GameMode::PLAY;

        whiteKingMoved = false;
        blackKingMoved = false;
        whiteRightRookMoved = false;
        blackRightRookMoved = false;
        whiteLeftRookMoved = false;
        blackLeftRookMoved = false;

        created = false;

        fields.clear();
        fields.push_back(currentState());
        fieldsIndex = 0;

    }

    void Game::updateKingPosition() {
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

    Game::GameState Game::currentState() const {
        return GameState{
            gMap,
            whiteKingMoved,
            blackKingMoved,
            whiteRightRookMoved,
            blackRightRookMoved,
            whiteLeftRookMoved,
            blackLeftRookMoved
        };
    }

    void Game::loadState(const GameState& state) {
        gMap = state.board;

        whiteKingMoved = state.whiteKingMoved;
        blackKingMoved = state.blackKingMoved;
        whiteRightRookMoved = state.whiteRightRookMoved;
        blackRightRookMoved = state.blackRightRookMoved;
        whiteLeftRookMoved = state.whiteLeftRookMoved;
        blackLeftRookMoved = state.blackLeftRookMoved;

        updateKingPosition();
    }


    void Game::rebuildActualLogs() {
        actualLogs.clear();

        int start = std::max(0, static_cast<int>(logList.size()) - 10);

        for (int i = start; i < static_cast<int>(logList.size()); ++i) {
            actualLogs.push_back(i);
        }
    }

    void Game::moveFigure() {
        // Figur anklicken, dann sollen alle moeglichen Felder in ROT angezeigt werden
        // Auf die Felder dann klicken zum verschieben

        // Erreichbare Felder haben entweder den Zustand Players::EMPTY oder Players::wasAnderes
        // Bei Players::EMPTY soll eine andere Farbe angezeigt werden als bei Players::wasAnderes


        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();


            // Maus hat aufs Spielfeld geklickt
            if (mousePos.x > static_cast<float>(fieldX) && mousePos.x < static_cast<float>(fieldX + fieldlength)
                && mousePos.y > static_cast<float>(fieldY) && mousePos.y < static_cast<float>(fieldY + fieldlength)) {

                // Identifizieren des geklickten Blocks
                float nx = mousePos.x - static_cast<float>(fieldX);
                float ny = mousePos.y - static_cast<float>(fieldY);

                int x = static_cast<int>(nx / static_cast<float>(blocksize));
                int y = static_cast<int>(ny / static_cast<float>(blocksize));


                auto [bx, by] = viewToBoard(x, y);
                x = bx;
                y = by;


                // Wenn das angeklickte Feld schon angeklickt war, wird die Markierung weggemacht
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

                                    lostBlackPieces.push_back(static_cast<Players>(gMap[move.second][move.first]));

                                    if (lostBlackPieces.size() > 20) {
                                        lostBlackPieces.pop_front();
                                    }

                                }
                                else {
                                    std::cout << "---\nWhite lost a figure\n---" << std::endl;
                                    lostWhitePieces.push_back(static_cast<Players>(gMap[move.second][move.first]));

                                    if (lostWhitePieces.size() > 20) {
                                        lostWhitePieces.pop_front();
                                    }

                                }
                            }

                            found = true;

                            // Weisser Koenig will kurze Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::W_KING) && move.second == 7 && move.first == 6) {
                                gMap[7][5] = static_cast<int>(Players::W_ROOK);
                                gMap[7][7] = static_cast<int>(Players::EMPTY);
                            }

                            // Schwarzer Koenig kurze Rochade
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::B_KING)
                                && move.second == 0 && move.first == 6) {
                                gMap[0][5] = static_cast<int>(Players::B_ROOK);
                                gMap[0][7] = static_cast<int>(Players::EMPTY);
                                }

                            // Weisser Koenig will lange Rochade machen
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::W_KING) && move.second == 7 && move.first == 2) {
                                gMap[7][3] = static_cast<int>(Players::W_ROOK);
                                gMap[7][0] = static_cast<int>(Players::EMPTY);
                            }

                            // Schwarzer Koenig lange Rochade
                            if (gMap[clickedField.second][clickedField.first] == static_cast<int>(Players::B_KING)
                                && move.second == 0 && move.first == 2) {
                                gMap[0][3] = static_cast<int>(Players::B_ROOK);
                                gMap[0][0] = static_cast<int>(Players::EMPTY);
                                }


                            // Endposition wird auf die Spielerindex gesetzt
                            bool hit = false;
                            if (isEnemyPiece(static_cast<Players>(gMap[move.second][move.first]))) {
                                hit = true;
                            }

                            auto movingPiece = static_cast<Players>(gMap[clickedField.second][clickedField.first]);

                            if (movingPiece == Players::W_KING) {
                                whiteKingMoved = true;
                            }
                            else if (movingPiece == Players::B_KING) {
                                blackKingMoved = true;
                            }
                            else if (movingPiece == Players::W_ROOK) {
                                if (clickedField.first == 0 && clickedField.second == 7) {
                                    whiteLeftRookMoved = true;
                                }
                                else if (clickedField.first == 7 && clickedField.second == 7) {
                                    whiteRightRookMoved = true;
                                }
                            }
                            else if (movingPiece == Players::B_ROOK) {
                                if (clickedField.first == 0 && clickedField.second == 0) {
                                    blackLeftRookMoved = true;
                                }
                                else if (clickedField.first == 7 && clickedField.second == 0) {
                                    blackRightRookMoved = true;
                                }
                            }

                            gMap[move.second][move.first] = gMap[clickedField.second][clickedField.first];

                            auto player = static_cast<Players>(gMap[clickedField.second][clickedField.first]);

                            bool shortCastle =
                                (player == Players::W_KING || player == Players::B_KING) &&
                                clickedField.first == 4 &&
                                move.first == 6;

                            bool longCastle =
                                (player == Players::W_KING || player == Players::B_KING) &&
                                clickedField.first == 4 &&
                                move.first == 2;

                            // Spielzug loggen
                            if (shortCastle) {
                                logList.emplace_back(player, std::make_tuple(-1, -1, false)); // 0-0
                            }
                            else if (longCastle) {
                                logList.emplace_back(player, std::make_tuple(-2, -2, false)); // 0-0-0
                            }
                            else {
                                logList.emplace_back(player, std::make_tuple(move.first, move.second, hit));
                            }


                            // Mehr Elemente sind in logList als angezeigt werden koennen
                            rebuildActualLogs();


                            // Ursprungsposition wird auf 0 gesetzt (EMPTY)
                            gMap[clickedField.second][clickedField.first] = static_cast<int>(Players::EMPTY);

                            // Markierung nach erfolgreichem Zug wegmachen
                            this->clickedField = {-1, -1};

                            // Letzten Zug merken
                            lastMove.first = move.first;
                            lastMove.second = move.second;


                            // Wenn auf der obersten Linie ein weisser Bauer steht,  soll das Fenster aufgerufen werden
                            // Das gleiche gilt fuer den schwarzen Bauern auf der untersten Linie

                            for (int i = 0; i < 8; i++) {
                                if (gMap[0][i] == static_cast<int>(Players::W_PAWN) || gMap[7][i] == static_cast<int>(Players::B_PAWN)) {
                                    gameMode = GameMode::CHOOSE_MODE;
                                }
                            }


                            // Spielerwechsel
                            turn = !turn;

                            if (fieldsIndex + 1 < fields.size()) {
                                fields.erase(fields.begin() + fieldsIndex + 1, fields.end());
                                recoveryLogList = std::stack<std::pair<Players, std::tuple<int,int, bool>>>();

                                recoveryLostBlackPieces = std::stack<Players>();
                                recoveryLostWhitePieces = std::stack<Players>();

                                rebuildActualLogs();
                            }

                            // Snapshot vom Feld speichern
                            fields.push_back(currentState());

                            fieldsIndex++;

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

    bool Game::isThreatend(int x, int y, bool byWhite) {
        std::vector<std::pair<int, int>> enemyMoves;

        for (int yy = 0; yy < 8; yy++) {
            for (int xx = 0; xx < 8; xx++) {
                auto p = static_cast<Players>(gMap[yy][xx]);

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


    bool Game::wouldKingBeThreatenedAfterMove(int fromX, int fromY, int toX, int toY, Players king) {
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
    bool Game::hasAnyLegalMove(bool white) {

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {

                auto p = static_cast<Players>(gMap[y][x]);

                if (white && !isWhitePiece(p)) continue;
                if (!white && !isBlackPiece(p)) continue;

                std::vector<std::pair<int,int>> moves;
                getPossibleMoves(p, x, y, moves);

                for (const auto& move : moves) {

                    auto target = static_cast<Players>(gMap[move.second][move.first]);

                    // Eigene Figuren dürfen nicht geschlagen werden
                    if (white && isWhitePiece(target)) continue;
                    if (!white && isBlackPiece(target)) continue;

                    int captured = gMap[move.second][move.first];

                    gMap[move.second][move.first] = gMap[y][x];
                    gMap[y][x] = static_cast<int>(Players::EMPTY);

                    updateKingPosition();

                    bool kingInCheck = white
                        ? isThreatend(whiteKingPosition.first, whiteKingPosition.second, false)
                        : isThreatend(blackKingPosition.first, blackKingPosition.second, true);

                    gMap[y][x] = static_cast<int>(p);
                    gMap[move.second][move.first] = captured;

                    updateKingPosition();

                    if (!kingInCheck) {
                        return true;
                    }
                }
            }
        }

        return false;
    }



    void Game::calculatePossibleMoves() {

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
                auto newPlayer = static_cast<Players>(gMap[newy][newx]);

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

        bool whiteInCheck = isThreatend(
            whiteKingPosition.first,
            whiteKingPosition.second,
            false
        );

        bool blackInCheck = isThreatend(
            blackKingPosition.first,
            blackKingPosition.second,
            true
        );

        // Weiß ist am Zug
        if (turn) {

            bool whiteHasLegalMove = hasAnyLegalMove(true);

            if (whiteInCheck && !whiteHasLegalMove) {
                check = Check::CHECK_NONE;
                won = Won::WON_BLACK;
            }
            else if (!whiteInCheck && !whiteHasLegalMove) {
                check = Check::CHECK_NONE;
                won = Won::DRAW; // Patt
            }
            else if (whiteInCheck) {
                check = Check::CHECK_BLACK;
                won = Won::NONE;
            }
            else {
                check = Check::CHECK_NONE;
                won = Won::NONE;
            }
        }

        // Schwarz ist am Zug
        else {

            bool blackHasLegalMove = hasAnyLegalMove(false);

            if (blackInCheck && !blackHasLegalMove) {
                check = Check::CHECK_NONE;
                won = Won::WON_WHITE;
            }
            else if (!blackInCheck && !blackHasLegalMove) {
                check = Check::CHECK_NONE;
                won = Won::DRAW; // Patt
            }
            else if (blackInCheck) {
                check = Check::CHECK_WHITE;
                won = Won::NONE;
            }
            else {
                check = Check::CHECK_NONE;
                won = Won::NONE;
            }
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

    void Game::getPawnAttackMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec) {
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


    void Game::getKingAttackMoves(Players p, int x, int y, std::vector<std::pair<int,int>>& moves) {
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

    void Game::getPossibleMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec) {

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
                if (y > 0) {
                    // Wenn Feld frei ist
                    if (isEmpty(static_cast<Players>(gMap[y-1][x]))) {
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
                if (y < 7) {
                    if (isEmpty(static_cast<Players>(gMap[y+1][x]))) {
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


    void Game::drawPossibleMoves() const {
        for (auto move : possibleMoves) {

            auto [vx, vy] = boardToView(move.first, move.second);

            int nx = fieldX + (blocksize * vx);
            int ny = fieldY + (blocksize * vy);

            DrawRectangle(nx, ny, blocksize, blocksize, {140,0,0,255});
            DrawRectangleLines(nx, ny, blocksize, blocksize, BLACK);
        }
    }


bool Game::created = false;
