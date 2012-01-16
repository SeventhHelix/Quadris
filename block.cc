#include "block.h"
#include <cstddef>

// pieceCount denotes the number of Blocks remaining within this piece
Block::Block(Block** &peers, int *numPeers, char level, char type, int x, int y, int width, int height) {
    
    this->peers = peers;
    this->peerPosn = *numPeers;
    this->peers[*numPeers] = this;
    (*numPeers)++;
    this->peerCount = numPeers;
    this->type = type;
    this->level = level;
    this->coord.x = x;
    this->coord.y = y;
    this->width = width;
    this->height = height;
}

Block::~Block() {
    *(this->peerCount) -= 1;
    this->peers[this->peerPosn] = NULL;

    // piece has been destroyed
    if (*(this->peerCount) == 0) {
        for (int i = 0; i < 4; i++) {
            delete this->peers[i];
        }
        delete[] this->peers;
        delete this->peerCount;
    }
}


// mutate coordinates in one call
void Coord::setCoord(int x, int y) {
    this->x = x;
    this->y = y;
}

// mutate coordinates in one call (polymorphism)
void Coord::setCoord(Coord c) {
    this->setCoord(c.x, c.y);
}
 

// Returns true iff this coordinate belongs to this piece
bool Block::coordBelongs(int x, int y) {
    for (int i = 0; i < 4; i++) {
        if (this->peers[i]->coord.x == x && this->peers[i]->coord.y == y) {
            return true;
        }
    }
    return false;
}

bool Block::coordBelongs(Coord c) {
    return this->coordBelongs(c.x, c.y);
}

// Returns the block type
char Block::getType() {
    return this->type;
}

// Returns the blocks peers
Block** Block::getPeers() {
    return this->peers;
}

// Returns the number of peers this block has
int Block::getPeerCount() {
    return *(this->peerCount);
}

// Returns the difficulty level this block was created on
char Block::getLevel() {
    return this->level;
}

// Returns the block's coord struct
Coord Block::getCoord() {
    return this->coord;
}


// Sets the blocks coordinates
void Block::setCoord(int x, int y) {
   this->coord.x = x;
   this->coord.y = y;
}

void Block::setCoord(Coord c) {
    this->setCoord(c.x, c.y);
}


// Sets the new width/height of the piece
void Block::setDimensions(int w, int h) {
    this->width = w; this->height = h;
}

int Block::getWidth() {
    return this->width;
}

int Block::getHeight() {
    return this->height;
}
