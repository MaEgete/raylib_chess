#pragma once

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

    // SaveLog Button
    Rectangle saveLogButton;

    Rectangle loadLogButton;

    Rectangle goBackButton;
    Rectangle goForwardButton;



    // PLAY = Spielen
    // CHOOSE_MODE = Figur austauschen mit Queen, Turm, Springer, Laufer
    enum class GameMode {
        PLAY,
        CHOOSE_MODE,
        END,
    };

    enum class Won {
        WON_WHITE,
        WON_BLACK,
        NONE,
        DRAW,
    };

    enum class Check {
        CHECK_WHITE,
        CHECK_BLACK,
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

    using Board = std::array<std::array<int, 8>, 8>;

    struct GameState {
        Board board;
        bool whiteKingMoved;
        bool blackKingMoved;
        bool whiteRightRookMoved;
        bool blackRightRookMoved;
        bool whiteLeftRookMoved;
        bool blackLeftRookMoved;
    };

    Board gMap{{
        {9 ,11,10,8 ,7 ,10,11,9},
        {12 ,12 ,12 ,12 ,12 ,12 ,12 ,12},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
        {6 ,6 ,6 ,6 ,6 ,6 ,6 ,6},
        {3 ,5 ,4 ,2 ,1 ,4 ,5 ,3},
    }};


    // Angeklicktes Feld markieren
    std::pair<int, int> clickedField = {-1, -1};

    // Alle moeglichen Zuege markieren
    std::vector<std::pair<int, int>> possibleMoves;

    // true == weiss ist am Zug
    // false == schwarz ist am Zug
    bool turn = true;



    // Maximale Groesse 20 Stueck
    std::list<Players> lostBlackPieces{};
    std::stack<Players> recoveryLostBlackPieces{};

    std::list<Players> lostWhitePieces{};
    std::stack<Players> recoveryLostWhitePieces{};


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


    bool whiteKingMoved = false;
    bool blackKingMoved = false;
    bool whiteRightRookMoved = false;
    bool blackRightRookMoved = false;
    bool whiteLeftRookMoved = false;
    bool blackLeftRookMoved = false;


    Rectangle logRectangle;

    // Eine Liste, wo einem Spieler ein Zug zugewiesen wird
    // int, int = Feld auf das gezogen wurde
    // bool = ob auf dem Feld ein gegnerischer Spieler stand
    std::vector<std::pair<Players, std::tuple<int,int, bool>>> logList;

    std::stack<std::pair<Players, std::tuple<int,int, bool>>> recoveryLogList;


    // Die max 5 Logs die angezeigt werden
    // Es sollen die indizes von logList hier gespeichert werden
    std::list<int> actualLogs;

    static bool created;

    size_t fieldsIndex = 0;
    std::vector<GameState> fields;


public:

    Game(int fieldlength = 600);

    ~Game();

    static bool mouseCollision(Vector2 mousePos, Rectangle rec);

    void loop();

    void loadLog();

    // Spiel zeichnen
    void draw();

    void drawLoadLogButton();

    void drawGoForwardButton();

    void drawGoBackButton();

    void drawSaveLogButton();


    void drawRestartButton();

    bool isBoardFlipped() const;

    std::pair<int, int> boardToView(int x, int y) const;

    std::pair<int, int> viewToBoard(int x, int y) const;


    // Regeln
    void update();

    static std::string playerToString(const Players& player);

    static std::string coordinatesToLabels(int x, int y);


    void saveLogs();



    void scrollLogList();


    void drawLogList();


    void drawEndField() const;


    void drawChooseField();

    static void drawFiguresLost(std::vector<Players>& vect, int blackXoff = 0, int whiteXoff = 0, int fontSize = 30, int offset = 1, int yOff = 0, bool background = false, Color backgroundColor = GREEN);

    //Wenn nur noch ein Spieler auf dem Feld ist und dieser keinen PossibleMove hat, dann Schachmatt!

    void drawLostFigures();



    void drawText() const;

    // Spielfeld zeichnen
    void drawField() const;

    void drawLabels() const;

    void drawClickedField();



    void drawFigures();



    // Returns False when player is an empty grid
    bool isEnemyPiece(const Players& player);

    static bool isEmpty(const Players& player);

    // Returns True when player is a black piece
    static bool isBlackPiece(const Players& player);

    static bool isWhitePiece(const Players& player);


    void restart();

    void updateKingPosition();

    GameState currentState() const;

    void loadState(const GameState& state);


    void rebuildActualLogs();

    void moveFigure();

    bool isThreatend(int x, int y, bool byWhite);

    bool wouldKingBeThreatenedAfterMove(int fromX, int fromY, int toX, int toY, Players king);


    // Prüft, ob es IRGENDEINEN legalen Zug gibt,
    // der das aktuelle Schach aufhebt
    bool hasAnyLegalMove(bool white);



    void calculatePossibleMoves();

    static void getPawnAttackMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec);


    static void getKingAttackMoves(Players p, int x, int y, std::vector<std::pair<int,int>>& moves);

    void getPossibleMoves(Players player, int x, int y, std::vector<std::pair<int, int>>& vec);


    void drawPossibleMoves() const;


};


