#include "trie.h"
#include "code.h"
#include <stdio.h>
#include <stdlib.h>

TrieNode *trie_node_create(uint16_t code) {
    TrieNode *node = malloc(sizeof(*node));
    // printf("%lu \n", sizeof (TrieNode));
    node->code = code;
    for (int i = 0; i < 256; i++) {
        node->children[i] = NULL;
    }

    return node;
}

void trie_node_delete(TrieNode *n) {
    free(n);
}

TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    return root;
}

void trie_reset(TrieNode *root) {

    for (int i = 0; i < ALPHABET; i++) {
        root->children[i] = NULL;
    }
}

void trie_delete(TrieNode *n) {
    for (int i = 0; i < ALPHABET; i++) {
        if (n->children[i] != NULL) {
            trie_delete(n->children[i]);
            // trie_node_delete(n->children[i]);
        }
    }
    trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return (n = (n == NULL || n->children[sym] == NULL ? NULL : n->children[sym]));
}
