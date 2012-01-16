#include <iostream>
#include "trie.h"

TrieNode::TrieNode(int p, int commandOffset) {
    posn = p;
    isWord = false;
    this->commandOffset = commandOffset;
    for (int i = 0; i < NumChars; i += 1) {
        this->letters[i] = NULL;
    } // for
}

TrieNode::~TrieNode() {
    for (int i = 0; i < NumChars; i++) {
        if (letters[i] != NULL) {
            delete letters[i];
        }
    }
}

void TrieNode::insert(const std::string &word, int functionIndex) {

    TrieNode* currentNode = this;

    for (int i = 0; i < int(word.length()); i++) {
        int charPosn = int(word[i])-97;
        if (currentNode->letters[charPosn] == NULL){
            currentNode->letters[charPosn] = new TrieNode(charPosn, functionIndex);
        } else {
            currentNode->commandOffset = -1;
        }
        currentNode = currentNode->letters[charPosn];
    }

    currentNode->commandOffset = functionIndex;
    return;
}

void TrieNode::remove(const std::string &word) {

    TrieNode* currentNode = this;

    for (int i = 0; i < int(word.length()); i++) {
        currentNode = currentNode->letters[int(word[i])-97];
    }
    currentNode->commandOffset = -1;
    return;
}


// transverse to possible tab completions
int TrieNode::find(const std::string &word) {

    std::string soFar;
    const TrieNode* currentNode = this;
    for (int i = 0; i < word.length(); i++) {
        int charPosn = int(word[i])-97;
        if (currentNode->letters[charPosn] == NULL) {
            return -2;
        } else {
            currentNode = currentNode->letters[charPosn];
        }
    }
    return currentNode->commandOffset;
}

