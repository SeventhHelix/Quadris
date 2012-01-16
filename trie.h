#ifndef __TRIE_H__
#define __TRIE_H__
#include <string>

class TrieNode {

    enum { Apostrophe = 26, NumChars = 27 };
    bool isWord;
    int posn;
    int commandOffset;
    TrieNode *letters[NumChars];

    public:
    TrieNode(int p, int commandOffset);
    ~TrieNode();

    void insert(const std::string &word, int functionIndex);
    void remove(const std::string &word );
    int find(const std::string &word );

}; // TrieNode


#endif
