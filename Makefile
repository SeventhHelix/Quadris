CXX = g++
CXXFLAGS = -Wall -MMD -L/usr/X11R6/lib -lX11 -std=c++0x
EXEC = quadris
OBJECTS = main.o block.o interpreter.o board.o PRNG.o trie.o window.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
