#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "io.h"
#include "code.h"
#include "trie.h"
#include "endian.h"

#define BITS 8

int bitlen(uint32_t code) {
    int len = 0;
    while (code > 0) {
        len++;
        code >>= 1;
    }
    return len;
}

int main(int argc, char *argv[]) {
    int options;

    int infile = STDIN_FILENO; //default
    int outfile = STDOUT_FILENO; //default
    bool verbose = false;

    while ((options = getopt(argc, argv, "vi:o:")) != -1) {
        switch (options) {

        case 'v': {
            verbose = true; //same as encode
            break;
        }
        case 'i': {
            if ((infile = open(optarg, O_RDONLY)) == -1) {
                fprintf(stderr, "%s : failed to open input\n", optarg);
                close(outfile);
                exit(EXIT_FAILURE);
            }

            break;
        }
        case 'o': {

            if ((outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
                fprintf(stderr, "%s : failed.\n", optarg);
                close(infile);
                exit(EXIT_FAILURE);
            }

            break;
        }
        default: { //default prints usage
            fprintf(stderr,
                "Usage : "
                "%s -i <infile> -o <outfile> -v (optional stats)\n",
                argv[0]);
            exit(EXIT_FAILURE);
        }
        }
    }

    //FilePermissions
    FileHeader head;
    read_header(infile, &head);
    fchmod(outfile, head.protection);

    //DECOMPRESSION pseudocode from asgn doc, elaborated on design doc
    WordTable *table = wt_create(); //create word table
    uint16_t curr_code = 0; //empty code
    uint8_t curr_sym = 0; //empty code
    uint16_t next_code = START_CODE; //start at first usable code

    while (read_pair(infile, &curr_code, &curr_sym, bitlen(next_code)) == true) { //while readable
        //use next code as index and append the current code/symbol
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]); //write decoded word
        next_code++; //next code +=1 to iterate through the encoded values
        if (next_code == MAX_CODE) { //if at the end reset
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile); //write and clear words from buffer

    if (verbose == 1) {
        {
            double spacesave = 100.0 * (1 - ((double) total_bits / (double) (total_syms * BITS)));
            fprintf(stderr, "Compressed file size: %lu bytes\n", (unsigned long) total_bits / BITS);
            fprintf(stderr, "Uncompressed file size: %lu bytes\n", (unsigned long) total_syms);
            fprintf(stderr, "Space saved: %.2f%%\n", spacesave);
        }
    }

    return 0;
}
