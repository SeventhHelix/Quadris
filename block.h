#ifndef __BLOCK_H__
#define __BLOCK_H__

struct Coord {
    int x, y;
    void setCoord(int x, int y);
    void setCoord(Coord c);
};


class Block {
    char level, type;
    int peerPosn, width, height;
    int *peerCount;
    Block **peers;
    Coord coord;

    public:
    Block(Block** &peers, int *numPeers, char difficulty, char charType, int x, int y, int width, int height);
    ~Block();

    bool coordBelongs(int x, int y);
    bool coordBelongs(Coord c);
    
    // getters
    char getType();
    char getLevel();
    Block** getPeers();
    int getPeerCount();
    int getWidth();
    int getHeight();
    Coord getCoord();
    
    // setters
    void setCoord(int x, int y);
    void setCoord(Coord c);
    void setDimensions(int width, int height);


};

#endif
