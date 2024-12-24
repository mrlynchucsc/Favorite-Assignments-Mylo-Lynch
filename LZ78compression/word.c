#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "word.h"
#include "code.h"
#include <stdio.h>

Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = (Word *) calloc(1, sizeof(Word)); //Allocate memory and set to 0
    if (w != NULL) {
        w->syms = (uint8_t *) calloc(len, sizeof(uint8_t)); // Allocate memory and set to 0
        if (w->syms != NULL) {
            w->len = len;
            memcpy(w->syms, syms, len * sizeof(uint8_t)); //copy memory
        } else {
            free(w);
            w = NULL;
        }
    }
    return w;
}

Word *word_append_sym(Word *w, uint8_t sym) {
    uint8_t *newsyms
        = (uint8_t *) malloc((w->len + 1) * sizeof(uint8_t)); // Allocate memory for new symbols
    memcpy(newsyms, w->syms, w->len * sizeof(uint8_t)); //Copy old
    newsyms[w->len] = sym; //append new
    Word *newword = word_create(newsyms, w->len + 1); //new word with new symbols
    free(newsyms); // Free unnecessary memory
    return newword; //new word with appended symbol
}

void word_delete(Word *w) {
    free(w->syms); //be free
    free(w); //goodbye w
}

WordTable *wt_create(void) { //creates a word table
    //be sure to allocate memory
    WordTable *wt = (WordTable *) malloc((MAX_CODE) * sizeof(Word *));
    if (wt != NULL) { //if its not null then it should create an empty word for the empty code
        wt[EMPTY_CODE] = word_create(NULL, 0);
        for (uint16_t i = 0; i < MAX_CODE; i++) {
            if (i != EMPTY_CODE) {
                wt[i] = NULL;
            }
        }
    }
    return wt;
}

void wt_reset(WordTable *wt) { //resets a tables values to NULL
    for (int i = START_CODE; i < MAX_CODE;
         i++) { //from start to max because thats the max possible size
        if (wt[i] != NULL) { //if not null
            word_delete(wt[i]); //be free!
            wt[i] = NULL; //be nothing
        }
    }
}

void wt_delete(WordTable *wt) {
    free(wt); //be free!
}
