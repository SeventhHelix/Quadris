#include <iostream>
#include "interpreter.h"
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {

    Interpreter *intpt;

    int seed = 0;
    bool textOnly = false;

    // Gets command-line args
    for (int i=1; i < argc; i++) {
        if (string(argv[i]) == "-text") textOnly = true;
        if (string(argv[i]) == "-seed") stringstream(argv[i+1]) >> seed;
    }

    intpt = new Interpreter(cin, textOnly, seed);

    // Starts game
    // Invokes board start game and waits for input
    intpt->beginGame();


    delete intpt;

    return 0;
}
