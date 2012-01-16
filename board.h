#ifndef __BOARD_H__
#define __BOARD_H__

#include "block.h"
#include "board.h"
#include "PRNG.h"
#include "window.h"
#include <iostream>


class Interpreter;  // Forward declaration. Avoids the circular include

class Board {
    Block ***board;     // the main 2D board of block pointers
    Block *activePiece; // top left square of the active Piece
    Interpreter *intpt;
    Xwindow *winGrid, *winStats; 
    int level, score, hiScore;
    char nextPiece;
    bool textOnly, firstBoard, genFirstPiece, pieceCleared;
    PRNG *prng;
    
    // Window drawing functions
    void winRedrawStats(int width, int height, int fgColor, int bgColor);
    void winClearBlock(int x, int y, Xwindow *xwin);
    void winDrawBlock(int x, int y, char type, Xwindow *xwin);
    int getBlockColour(char type);

    // cell emptiers, setters, and movers
    void emptyCell(Coord c);
    void emptyCell(int x, int y);
    void setCell(Block *b, int x, int y);
    void setCell(Block *b, Coord c);
    void moveBlock(Block *b, int x, int y);
    void moveBlock(Block *b, Coord c);

    public:
    Board(bool textOnly, int seed, int hiScore, Interpreter *intpt);
    ~Board();

    bool addPiece();
    void generatePieceType();
    void reInitializeBoard();
    void startGame();

    // transformations on the active pice
    bool moveActivePiece(char dir);
    bool rotateActivePiece(char dir);
    bool dropActivePiece();

    // Window drawing functions
    void winClearActivePiece();
    void winDrawActivePiece();
    void winRedrawGrid(int width, int height, int fgColor, int bgColor);

    // manipulate the difficulty
    void changeLevel(char dir);

    // Gets the HiScore
    //int getHiScore();
    void setPieceCleared(bool val);

    // output  the board
    //void printBoard(std::ostream &out);
    //void printNext(std::ostream &out);
    friend std::ostream& ::operator<<(std::ostream &out, const Board &brd); 
};
#endif
