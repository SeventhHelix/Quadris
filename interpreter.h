#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <fstream>
#include <vector>
#include "trie.h"
#include "board.h"

typedef bool (*rptr)(char);

class Interpreter {

    std::string currentCommand;
    int commandMultiplier, seqIndex, seed;
    bool textOnly;

    TrieNode *dictionary;
    Board *board;
    std::ifstream ifs;

    public:
    Interpreter(std::istream &i, bool textOnly, int seed);
    ~Interpreter();

    void setCommands(std::string *dict, int dictLen);
    void beginGame(); 
    void execCmd();
    char readPieceType();
    void renameCmd();

};

#endif
