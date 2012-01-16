#include "interpreter.h"
#include "board.h"
#include "math.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <unistd.h> // For usleep

using namespace std;

// Board Constructor
Board::Board(bool textOnly, int seed, int hiScore, Interpreter *intpt) {
    this->level = 1;
    this->score = 0;
    this->hiScore = hiScore;
    this->textOnly = textOnly;
    this->intpt = intpt;
    this->firstBoard = true;
    this->pieceCleared = false;

    // Prevents creating a new PRNG with seed 0
    if (seed != 0) this->prng = new PRNG(seed);
    else this->prng = new PRNG();

    // Initializes the base array since this isn't done in reInitializeBoard
    this->board = new Block**[18];

    // Finishes the initialization of the board
    // This is also called to reset certain parts of the board
    this->reInitializeBoard();

    // Indicates reInitializeBoard can delete parts of the array
    this->firstBoard = false;

    // Indicates whether or not to draw the piece in winStats
    this->genFirstPiece = false;

}


// Board destructor
Board::~Board() { 

    // Deletes the entire grid of block pointers and sets each cell to NULL
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 10; x++) {
            delete this->board[y][x];
            this->board[y][x] = NULL;
        }
        delete[] this->board[y];
    }
    delete[] this->board;
    delete this->prng;

    delete this->winGrid;
    delete this->winStats;
}


/*
 *
 * Window Redrawing Methods
 *
 */

// Reinitializes the board
// Clears the grid, but keeps the same PRNG, hiScore, etc.
void Board::reInitializeBoard() {

    // Only deletes if this isn't the first board - ie. being reset
    if (!(this->firstBoard)) {
        for (int y = 0; y < 18; y++) {
            for (int x = 0; x < 10; x++) {
                delete this->board[y][x];
                this->board[y][x] = NULL;
            }
            delete[] this->board[y];
        }
    }

    this->score = 0;
    this->genFirstPiece = true;

    // Sets the new block pointers 
    // Note that the main Block** array is still available so we don't need to re-new it
    for (int i=0; i < 18; i++) {
        this->board[i] = new Block*[10];
        for (int j=0; j < 10; j++) {
            this->board[i][j] = NULL;
        }
    }

    // Initializes and displays the XWindows if not text-only mode
    if (!textOnly) {
        this->winRedrawGrid(251, 451, 1, 0);
        this->winRedrawStats(150,180, 1, 0);
    }

    // Generates the next piece and adds it
    // addPiece will also generate a new piece, which will be displayed when the 
    // board is printed
    this->generatePieceType();
    bool addSucc = this->addPiece();
}

// Redraws the Grid window
void Board::winRedrawGrid(int width, int height, int bgColor, int fgColor) {
    bool redraw;

    if (this->winGrid == NULL) {
        this->winGrid = new Xwindow(width, height);
    }

    redraw = this->winGrid->isFirstDraw();

    // Draws the initial grid if needed
    if (redraw) {
        this->winGrid->fillRectangle(0, 0, width, height, fgColor);
        
        // Fixes the grid lines not being drawn on local machines
        usleep(15000); 

        // Horizontal grid lines
        for (int y = 0; y < 19; y++) {
        usleep(1000); 
            this->winGrid->fillRectangle(0, 25*y, 250, 1, bgColor);
        }

        // Vertical grid lines
        for (int x = 0; x < 11; x++) {
        usleep(1000); 
            this->winGrid->fillRectangle(25*x, 0, 1, 450, bgColor);
        }
    }

    // Redraws the blocks themselves if needed
    // Done when a line is cleared or the game is reset
    if (!redraw) {
        for (int y = 0; y < 18; y++) {
            for (int x = 0; x < 10; x++) {
                if (this->board[y][x] != NULL) {
                    this->winDrawBlock(x*25, y*25, this->board[y][x]->getType(), this->winGrid);
                } else {
                    this->winClearBlock(x*25, y*25, this->winGrid);
                }
            }
        }
    }

}

// Redraws the Stats window
void Board::winRedrawStats(int width, int height, int bgColor, int fgColor) {
    std::ostringstream oss;

    if (this->winStats == NULL) {
        this->winStats = new Xwindow(width, height);
    }

    // Clears the level/scores/next block from last Redraw
    this->winStats->fillRectangle(100, 0, 100, 100, 0);
    usleep(1000);
    this->winStats->fillRectangle(0, 100, 125, 100, 0);

    // Draws the initial text 
    // This ONLY needs to be done once - even if the board is reset
    if (this->winStats->isFirstDraw()) {
        this->winStats->fillRectangle(0, 0, width, height, 0);
        this->winStats->drawString(25, 20, "Level:", 1);
        this->winStats->drawString(25, 40, "Score:", 1);
        this->winStats->drawString(25, 60, "Hi Score:", 1);
        this->winStats->drawString(25, 80, "Next:", 1);
    }


    // Reads and displays the level/scores
    oss << this->level;
    this->winStats->drawString(100, 20, oss.str(), 2);
    oss.str("");

    oss << this->score;
    this->winStats->drawString(100, 40, oss.str(), 2);
    oss.str("");

    oss << this->hiScore;
    this->winStats->drawString(100, 60, oss.str(), 2);
}

// Clears a block on xwin at x and y
void Board::winClearBlock(int x, int y, Xwindow *xwin) {
    this->winDrawBlock(x, y, 'W', xwin);
}

// Clears the active piece on the grid window
void Board::winClearActivePiece() {
    if (!(this->textOnly)) {
        for (int i=0; i < 4; i++) {
            this->winClearBlock(this->activePiece->getPeers()[i]->getCoord().x*25, 
                    this->activePiece->getPeers()[i]->getCoord().y*25,
                    this->winGrid);
        }
    }
}

// Redraws the active piece on the grid window
void Board::winDrawActivePiece() {
    if (!(this->textOnly)) {
        for (int j = 0; j < 4; j++) {
            winDrawBlock(this->activePiece->getPeers()[j]->getCoord().x*25,
                    this->activePiece->getPeers()[j]->getCoord().y*25,
                    this->activePiece->getType(), this->winGrid);
        }
    }
}

// Draws the block on the specified window based on the type
void Board::winDrawBlock(int x, int y, char type, Xwindow *xwin) {
    int colour = this->getBlockColour(type);
    xwin->fillRectangle(x+1, y+1, 24, 24, colour);
}

// Takes a block type and returns the matching colour
int Board::getBlockColour(char type) {
    int colour;
    switch (type) {
        case 'W':
            colour = 0;
            break;
        case 'B':
            colour = 1;
            break;
        case 'I':
            colour = 2;
            break;
        case 'O':
            colour = 3;
            break;
        case 'J':
            colour = 4;
            break;
        case 'L':
            colour = 5;
            break;
        case 'S':
            colour = 6;
            break;
        case 'Z':
            colour = 7;
            break;
        case 'T':
            colour = 8;
            break;
    }

    return colour;
}




/*
 *
 * cell emptiers/getters/setters
 *
 */

// nullify the cell at (x,y)
void Board::emptyCell(int x, int y) {
    this->board[y][x] = NULL;
}

// nullify the cell at (x,y)
void Board::emptyCell(Coord c) {
    this->emptyCell(c.x, c.y);
}

// set a pointer at (x,y) to the given block
void Board::setCell(Block *b, int x, int y) {
    this->board[y][x] = b;
}

// set pointer at (c.x, c.y) to given block
void Board::setCell(Block *b, Coord c) {
    this->setCell(b, c.x, c.y);
}


// move the given block to (x,y), also nullify any references on
// the board to the block's old coordinates
void Board::moveBlock(Block *b, int x, int y) {
    this->emptyCell(b->getCoord());
    b->setCoord(x, y);
    this->setCell(b, x, y); 
}

void Board::moveBlock(Block *b, Coord c) {
    this->moveBlock(b, c.x, c.y);
}


// Sets if the active piece has been cleared or not
void Board::setPieceCleared(bool val) {
    this->pieceCleared = val;
}





/*
 * 
 * Board manipulation 
 * Piece movement commands
 *
 */

// Generates a piece type based on the board's difficulty setting
void Board::generatePieceType() {

    // Generates the random number
    unsigned int rNum = (*(prng))();

    char type;
    char typeArray[] = {'S','Z', 'I', 'J', 'L', 'O', 'T'};

    switch(this->level) {

        case 0: 
            type = this->intpt->readPieceType();
            // if readPieceType() returns '0', it falls through to level 1
            if (type != '0') break;
            this->changeLevel('+');
        case 1:
            if (rNum%12 <= 1) type = typeArray[rNum%12];
            else type = typeArray[rNum%5 + 2];
            break;

        case 2:
            type =  typeArray[rNum % 7];
            break;

        case 3:
            if (rNum%9 <= 4) type = typeArray[rNum%2];
            else type= typeArray[rNum%5 + 2];
            break;
    }

    this->nextPiece = type;

    if (!(this->textOnly) && !(this->genFirstPiece)) {
        int coords[8];
        switch (this->nextPiece) {
            case 'I':
                coords = { 0, 3, 1, 3, 2, 3, 3, 3 };
                break;
            case 'J':
                coords = { 0, 3, 0, 4, 1, 4, 2, 4 };
                break;
            case 'L':
                coords = { 2, 3, 0, 4, 1, 4, 2, 4 };
                break;
            case 'O':
                coords = { 0, 3, 1, 3, 0, 4, 1, 4 };
                break;
            case 'S':
                coords = { 1, 3, 2, 3, 0, 4, 1, 4 };
                break;
            case 'Z':
                coords = { 0, 3, 1, 3, 1, 4, 2, 4 };
                break;
            case 'T':
                coords = { 0, 3, 1, 3, 2, 3, 1, 4 };
                break;
        } 

        this->winRedrawStats(200,200, 0, 1);

        for (int i = 0; i < 8; i+=2) {
            this->winDrawBlock(coords[i] * 25 + 25, (coords[i+1]-3)*25 + 100, this->nextPiece, this->winStats);
        } // for

    } // if

    this->genFirstPiece = false;

}


// Adds the piece in the nextPiece field to the board
// returns true if the piece was successfully added to the board
bool Board::addPiece() {

    // creation code for piece of 'type'
    // directly access and initialize elements of board
    // that are affected by the new piece
    int coords[8];
    int width, height;

    switch (this->nextPiece) {
        case 'I':
            coords = { 0, 3, 1, 3, 2, 3, 3, 3 };
            width = 3; height = 0;
            break;
        case 'J':
            coords = { 0, 2, 0, 3, 1, 3, 2, 3 };
            width = 2; height = 1;
            break;
        case 'L':
            coords = { 2, 2, 0, 3, 1, 3, 2, 3 };
            width = 0; height = 1;
            break;
        case 'O':
            coords = { 0, 2, 1, 2, 0, 3, 1, 3 };
            width = 1; height = 1;
            break;
        case 'S':
            coords = { 1, 2, 2, 2, 0, 3, 1, 3 };
            width = 1; height = 1;
            break;
        case 'Z':
            coords = { 0, 2, 1, 2, 1, 3, 2, 3 };
            width = 2; height = 1;
            break;
        case 'T':
            coords = { 0, 2, 1, 2, 2, 2, 1, 3 };
            width = 2; height = 1;
            break;
    } 

    Block **p = new Block *[4];
    int *peerCount = new int;
    *peerCount = 0;

    // First checks to make sure each block can be added before placing them
    for (int i = 0; i < 8; i += 2) {
        if (board[coords[i+1]][coords[i]] != NULL) { // piece can't fit
            delete peerCount;
            delete[] p;
            throw "Game Over";
            return false;
        }
    }

    // If it's gotten here, it can safely add the block
    for (int i = 0; i < 8; i += 2) {
        this->board[coords[i+1]][coords[i]] = new Block(p, peerCount, this->level, 
                this->nextPiece, coords[i], coords[i+1], width, height);
    }
    
    // set the current active piece
    this->activePiece = p[0];

    if (!(this->textOnly)) {
        this->winDrawActivePiece();
    }

    this->generatePieceType();
    return true;
}


// Moves the active piece left right or down
bool Board::moveActivePiece(char dir) {

    int xdir = 0, ydir = 0, sp = 0, fp = 0;

    // Sets the change in directions and checks boundaries
    switch (dir) {
        case 'd':
            if (this->activePiece->getCoord().y+this->activePiece->getHeight()==17) {
                return false;
            }
            ydir = 1; 
            sp = 3;
            fp = -1;
            break;
        case 'l':
            if (this->activePiece->getCoord().x == 0) {
                return false;
            }
            xdir = -1; 
            sp = 0;
            fp = 4;
            break;
        case 'r':
            if (this->activePiece->getCoord().x+this->activePiece->getWidth()==9) {
                return false;
            }
            xdir = 1; 
            sp = 3;
            fp = -1;
            break;
    }


    // check to make sure the space is available,
    Block** thePiece = this->activePiece->getPeers();
    Coord newCoords[4]; 
    for (int i = 0; i < 4; i++) {
        Coord currentCoord = thePiece[i]->getCoord();
        newCoords[i].setCoord(currentCoord.x+xdir, currentCoord.y+ydir);

        // check to see if these coords are occupied
        if (this->board[newCoords[i].y][newCoords[i].x] != NULL &&
                !this->activePiece->coordBelongs(newCoords[i])) {
            return false;
        }
    }

    // apply the movement
    for (int i = 0; i < 4; i++) {
        // empty old cell
        this->emptyCell(thePiece[i]->getCoord());
        
        // set new coordinates
        thePiece[i]->setCoord(newCoords[i]);
    }
    for (int i = 0; i < 4; i++) {
        // assign new cells
        this->setCell(thePiece[i], thePiece[i]->getCoord());
    }

    return true; // move successful
}


// Drops the active piece to the bottom
// Checks and clears lines, and sets the score as appropriate
bool Board::dropActivePiece() {

    // Moves the piece to the bottom
    while (this->moveActivePiece('d'));

    // Redraws the piece, since Interpreter can only re-draw the next piece
    if (!(this->textOnly)) {
        this->winDrawActivePiece();
    }

    int linesCleared = 0;
    bool recheckLine = false;

    // Clears all full lines
    for (int y = 17; y > 2; y--) {
        bool lineCleared = false;

        // Rows need to be re-checked if they've been cleared
        // in case a row from above has fallen into place and needs to be cleared
        if (recheckLine) {
            y++;
            recheckLine = false;
        }

        for (int x = 0; x < 10; x++) {
            if (this->board[y][x] == NULL) {
                recheckLine = false;
                break;
            } else if (x == 9) {
                lineCleared = true;
            }
        }
        if (lineCleared) {
            lineCleared = false;
            recheckLine = true;
            linesCleared++; 

            // delete the row
            for (int x = 0; x < 10; x++) {
                // determine if piece bonus should be awarded
                if (this->board[y][x]->getPeerCount() == 1) {
                    this->score += pow(this->board[y][x]->getLevel()+1, 2);
                }
                delete this->board[y][x];
                this->board[y][x] = NULL;
                if (!(this->textOnly)) {
                    this->winClearBlock(x*25, y*25, this->winGrid);
                }
            }

            // Move the rest of the board downward into cleared spots
            // Starts one block above the cleared line (y-1) and moves up
            for (int r = y-1; r > 0; r--) {
                for (int i = 0; i < 10; i++) {
                    if (this->board[r][i] != NULL) {
                        this->moveBlock(this->board[r][i], i, r+1);
                        if (!(this->textOnly)) {
                            this->winDrawBlock(i*25, r*25, this->board[r+1][i]->getType(), 
                                               this->winGrid);
                        }
                    }
                }
            }
            recheckLine = true;
        } 

    }

    if (linesCleared > 0) {
        this->score += pow(linesCleared+this->level, 2);
    }
    if (this->score > this->hiScore) {
        this->hiScore = this->score;
    }

    if(!(this->addPiece())) {
        throw ("Game over");
    }

    return (linesCleared > 0);

}

// Rotates the active piece given a char indicating direction
// w - clockwise
// r - counter-clockwise
bool Board::rotateActivePiece(char dir) {

    Coord newCoords[4];
    int  cosTheta = 0,
         sinTheta = 1;

    Block **thePiece = this->activePiece->getPeers();

    switch (dir) {
        case 'w':
            cosTheta = 0;
            sinTheta = 1;
            break;
        case 'r':
            cosTheta = 0;
            sinTheta = -1;
            break;
    }

    Coord rotateCoord, rotateAxisCoord;
    rotateCoord.setCoord(thePiece[0]->getCoord());
    rotateAxisCoord.setCoord(thePiece[0]->getCoord());

    // determine the rotated coordinates BEFORE the translation
    for (int i = 0; i < 4; i++) {

        Coord currentIndexCoord = thePiece[i]->getCoord();

        if (dir == 'w') {

            // keep track of the bottom right
            if (rotateCoord.x < currentIndexCoord.x) {
                rotateCoord.x = currentIndexCoord.x;
            }
            if (rotateCoord.y < currentIndexCoord.y) {
                rotateCoord.y = currentIndexCoord.y;
            }

        } else if (dir == 'r') {

            // keep track of the top left
            if (rotateCoord.y > currentIndexCoord.y) {
                rotateCoord.y = currentIndexCoord.y;
            }
            if (rotateCoord.x > currentIndexCoord.x) {
                rotateCoord.x = currentIndexCoord.x;
            }
        }

        // keep track of the bottom-left
        if (rotateAxisCoord.y < currentIndexCoord.y) {
            rotateAxisCoord.y = currentIndexCoord.y;
        }
        if (rotateAxisCoord.x > currentIndexCoord.x) {
            rotateAxisCoord.x = currentIndexCoord.x;
        }

        newCoords[i].x = currentIndexCoord.x*cosTheta -\
                         currentIndexCoord.y*sinTheta;
        newCoords[i].y = currentIndexCoord.x*sinTheta +\
                         currentIndexCoord.y*cosTheta;
    }

    // transform the rotational coordinate
    Coord rotatedCoord;
    rotatedCoord.x = rotateCoord.x*cosTheta - rotateCoord.y*sinTheta;
    rotatedCoord.y = rotateCoord.x*sinTheta + rotateCoord.y*cosTheta;

    // find the difference between the translation after the rotation
    //Coord deltaCoord;
    int xdiff = rotateAxisCoord.x - rotatedCoord.x,
        ydiff = rotateAxisCoord.y - rotatedCoord.y;

    // apply the translation and check the new coordinates at the same time
    for (int i = 0; i < 4; i++) {

        newCoords[i].x += xdiff;
        newCoords[i].y += ydiff;
        // check to see if this block is occupied
        // TODO: ADD BOUNDARY CHECK
        //      SHOULD BE ABSTRACTED WITH SAME STUFF AS in board::moveActivePiece
        if (newCoords[i].y > 16 || newCoords[i].y < 0 || newCoords[i].x < 0 || 
            newCoords[i].x > 9 || this->board[newCoords[i].y][newCoords[i].x] != NULL &&
                !this->activePiece->coordBelongs(newCoords[i])) {
            return false;
        }
    }

    // keep track of the new top left piece, farthest right coordinate
    // and farthest bottom coordinate
    Block *topLeft = this->activePiece;
    int greatestX = 0, greatestY = 0;

    for (int i = 0; i < 4; i++) {

        this->emptyCell(thePiece[i]->getCoord());
        thePiece[i]->setCoord(newCoords[i]);

        // check if the current coord is farther left and above
        // we're keeping track of
        Coord currentPieceCoord = thePiece[i]->getCoord(),
              topLeftCoord = topLeft->getCoord();
        if ((currentPieceCoord.x < topLeftCoord.x  &&\
                    currentPieceCoord.y <= topLeftCoord.y ) ||
                ( currentPieceCoord.y < topLeftCoord.y) ){
            topLeft = thePiece[i];
        }
        if (currentPieceCoord.x > greatestX) {
            greatestX = currentPieceCoord.x;
        }
        if (currentPieceCoord.y > greatestY) {
            greatestY = currentPieceCoord.y;
        }
    }

    // update the active piece
    this->activePiece = topLeft;
    Coord topLeftCoord = topLeft->getCoord();
    for (int i = 0; i < 4; i++) {
        this->setCell(thePiece[i], thePiece[i]->getCoord());
        thePiece[i]->setDimensions(greatestX-topLeftCoord.x,greatestY-topLeftCoord.y);
    }

    return true;
}

// mutate level difficulty in a given direction
void Board::changeLevel(char dir) {
    switch (dir) {
        case '-':
            if (this->level != 0) this->level -= 1;
            break;
        case '+':
            if (this->level != 3) this->level += 1;
            break;
    }

    if (!(this->textOnly)) {
        this->winRedrawStats(200,200,1,0);
    }
}

// Board operator<< overload
std::ostream& operator<<(std::ostream &out, const Board &brd) {
    out << "Level:\t\t" << brd.level << std::endl; 
    out << "Score:\t\t" << brd.score << std::endl; 
    out << "Hi Score:\t" << brd.hiScore << std::endl; 

    out << "----------" << std::endl;
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 10; x++) {
            if (brd.board[y][x] != NULL) {
                out << brd.board[y][x]->getType();
            } else {
                out << " ";
            }
        }
        out << std::endl;
    }
    out << "----------" << std::endl;

    out << "Next:" << std::endl;

    // Prints the next piece to be added
    int coords[8];
    bool printed = false, printSpace = false;


    switch(brd.nextPiece) {
        case 'I':
            coords = { 0, 0, 1, 0, 2, 0, 3, 0 };
            break;
        case 'J':
            coords = { 0, 0, 0, 1, 1, 1, 2, 1 };
            break;
        case 'L':
            coords = { 2, 0, 0, 1, 1, 1, 2, 1 };
            break;
        case 'O':
            coords = { 0, 0, 1, 0, 0, 1, 1, 1 };
            break;
        case 'S':
            coords = { 1, 0, 2, 0, 0, 1, 1, 1 };
            break;
        case 'Z':
            coords = { 0, 0, 1, 0, 1, 1, 2, 1 };
            break;
        case 'T':
            coords = { 0, 0, 1, 0, 2, 0, 1, 1 };
            break;
    } 

    for (int y = 0; y < 2; y++) {
        if (printSpace) out << ' ';
        for (int x = 0; x < 4; x++) {
            for (int i = 0; i < 8; i+=2) {
                if (coords[i] == x && coords[i+1] == y) {
                    printed = true;
                    out << brd.nextPiece;
                }
            }
            if (!printed) {
                out << ' ';
                printed = false;
            }
        }
        if (brd.nextPiece == 'T' || brd.nextPiece == 'Z') printSpace = true;
        out << std::endl;
    }

    out << std::endl;
    return out;
}
