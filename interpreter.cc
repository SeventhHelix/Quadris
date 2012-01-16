#include "interpreter.h"
#include <iostream>

using namespace std;

// Interpreter constructor
Interpreter::Interpreter(std::istream &i, bool textOnly, int seed) {
    this->seqIndex = 0;
    this->ifs.open("sequence.txt");
    this->textOnly = textOnly;
    this->seed = seed;

    // initialize the dictionary
    this->dictionary = new TrieNode(0, -1);

    int numCommands = 10;
    std::string commands[10] = {"left", "right", "down", "drop", "levelup", 
        "leveldown", "restart", "clockwise", "counterclockwise", "rename"};

    // Sets the dictionary commands
    setCommands(commands, numCommands);

    // Creates the board and prints it out in its initial state
    this->board = new Board(textOnly, seed, 0, this);
    std::cout << *board;
}

// Interpreter destructor
Interpreter::~Interpreter() {
    delete this->board;
    delete this->dictionary;
}

// Main interpreter loop
// Loops over input, executing the respective board commands
void Interpreter::beginGame() {
    int numIndex;

    // Main game loop
    // Returns to main.cc once output is exhausted
    while (std::cin >> this->currentCommand) {
        this->commandMultiplier = 0;

        // Finds the index of the last digit
        numIndex = this->currentCommand.find_last_of("0123456789");
        if (numIndex == -1 || this->currentCommand == "restart") {
            this->commandMultiplier = 1;
        } else {
            for (int i = 0; i <= numIndex; i++) {
                this->commandMultiplier = this->commandMultiplier * 10 + this->currentCommand[i] - '0';
            }
        }

        // The command multiplier has been found
        // Executes the board command with the saved multiplier
        this->currentCommand = this->currentCommand.substr(numIndex + 1);
        this->execCmd();
    }

}

// Executes the given command 
void Interpreter::execCmd() {
    bool success = false;
    bool reDraw;
    bool reDrawGrid;

    // Check the dictionary for the command
    int cmdIndex = this->dictionary->find(this->currentCommand);

    // If the command is not found or is ambiguous, skip this check
    if (cmdIndex != -1 && cmdIndex != -2) {
        this->board->setPieceCleared(false);

        // Loops according to the given commandMultiplier
        for (int i=0; i < this->commandMultiplier; i++) {
            reDraw =(i+1 == this->commandMultiplier);
            reDrawGrid = false;

            // Executes the command based on the index returned by dictionary
            switch(cmdIndex) {

                // Move left
                case 0:
                    //(this.*objectInterface[0])('l'); break;
                    if (i == 0 && !textOnly) this->board->winClearActivePiece();
                    success = this->board->moveActivePiece('l'); 
                    if (reDraw && !textOnly ) this->board->winDrawActivePiece();
                    break;

                // Move right
                case 1: 
                    if (i == 0 && !textOnly) this->board->winClearActivePiece();
                    success = this->board->moveActivePiece('r');
                    if (reDraw && !textOnly) this->board->winDrawActivePiece();
                    break;

                // Move down
                case 2:
                    if (i == 0 && !textOnly) this->board->winClearActivePiece();
                    success = this->board->moveActivePiece('d');
                    if (reDraw && !textOnly) this->board->winDrawActivePiece();
                    break;

                // Drop piece
                case 3:
                    if (!textOnly) this->board->winClearActivePiece();
                    try {
                        reDrawGrid = this->board->dropActivePiece();
                        if (reDraw && !textOnly && reDrawGrid) this->board->winRedrawGrid(250,450, 1, 0);
                    } catch (...) {
                        std::cout << "Game over" << std::endl;
                        // Makes sure the loop is broken out of then restarts
                        i = this->commandMultiplier;
                        usleep(3000);
                        this->board->reInitializeBoard();
                    }
                    break;

                // Increase level
                case 4:
                    this->board->changeLevel('+'); break;

                // Decrease level
                case 5:
                    this->board->changeLevel('-'); break;

                // Restart
                case 6:
                    // Makes sure the loop is broken out of then restarts
                    i = this->commandMultiplier;
                    this->board->reInitializeBoard();
                    break;

                // Rotates clockwise
                case 7:
                    if (i == 0 && !textOnly) this->board->winClearActivePiece();
                    success = this->board->rotateActivePiece('w');
                    if (reDraw && !textOnly) this->board->winDrawActivePiece();
                    break;

                // Rotates counterclockwise
                case 8:
                    if (i == 0 && !textOnly) this->board->winClearActivePiece();
                    success = this->board->rotateActivePiece('r');
                    if (reDraw && !textOnly) this->board->winDrawActivePiece();
                    break;

                // Rename a command
                case 9:
                    this->renameCmd();
                    break;

            } // switch

            this->board->setPieceCleared(true);

        } // for 

        std::cout << *board;
    } else {
        switch (cmdIndex) {
            case -1:
                std::cout << "Ambiguous command entered." << std::endl;
                break;

            case -2:
                std::cout << "Command not found" << std::endl;
                break;
        }
    }
}


// Adds the commands in dict to the interpreter's dictionary
void Interpreter::setCommands(std::string *dict, int dictLen) {
    for (int i = 0; i < dictLen; i++ ) {
        this->dictionary->insert(dict[i], i);
    }
}

// Reads the next piece from sequence.txt if level is 0
char Interpreter::readPieceType() {
    char type;
    this->ifs >> type;

    // Returns 0 if ifs has gotten to EOF.
    // Otherwise this will return the last letter twice
    if (this->ifs.eof()) return '0';
    else return type;
}

// Reads an existing command and new command from input
// Renames an old command allowing the user to specify new commands
// Doesn't actually remove the old command - By design
void Interpreter::renameCmd() {
    string oldCmd;
    string newCmd;

    std::cin >> oldCmd;
    std::cin >> newCmd;

    // Inserts a new command with the new name, using the cmdIndex of 
    // the command being renamed
    this->dictionary->insert(newCmd, this->dictionary->find(oldCmd));

}
