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

//gets bit length
int bitlen(uint32_t code) {
    int len = 0;
    while (code > 0) {
        len++;
        code >>= 1;
    }
    return len;
}

int main(int argc, char *argv[]) {

    int options; // initialize options
    struct stat stats; //initialize stats for verbose

    bool verbose = false;
    int infile = STDIN_FILENO; //default
    int outfile = STDOUT_FILENO; //default

    while ((options = getopt(argc, argv, "vi:o:")) != -1) {
        switch (options) {

        case 'v': {
            verbose = true; //prints stats
            break;
        }
        case 'i': {
            if ((infile = open(optarg, O_RDONLY)) == -1) { //if failed
                fprintf(stderr, "%s : failed to open input file.\n", optarg);
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
        default: {
            fprintf(stderr,
                "Usage : " //prints usage by default
                "%s -i <infile> -o <outfile> -v (optional stats)\n",
                argv[0]);
            exit(EXIT_FAILURE);
        }
        }
    }

    //FilePermissions
    fstat(infile, &stats);
    fchmod(outfile, stats.st_mode);
    FileHeader *head = (FileHeader *) calloc(1, sizeof(FileHeader));
    head->magic = MAGIC;
    head->protection = stats.st_mode;
    write_header(outfile, head);

    //COMPRESSION from pseudocode on assignment doc, elaborated on design doc
    TrieNode *root = trie_create(); //create root
    TrieNode *curr_node = root; //set current node to root node
    TrieNode *prev_node = NULL; //no previous node since its the beginning of the code
    uint8_t curr_sym = 0; //current symbol is empty
    uint16_t prev_sym = 0; //no previous symbol
    uint32_t next_code = START_CODE; //start code is the first useable code (empty_code is first)

    while (read_sym(infile, &curr_sym)) { ///condition, while loop is happening while being read
        TrieNode *next_node
            = trie_step(curr_node, curr_sym); //step through the trie to encode the next word
        if (next_node
            != NULL) { //if not null then update prev and current node to the current node and next node respectively
            prev_node = curr_node;
            curr_node = next_node;
        } else { //word is complete
            write_pair(outfile, curr_node->code, curr_sym, bitlen(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code++;
            if (next_code == MAX_CODE) { //if max reached then reset tree from root
                trie_reset(root);
                curr_node = root;
                next_code = START_CODE;
            }
        }
        prev_sym = curr_sym;
    }

    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bitlen(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }

    write_pair(outfile, STOP_CODE, 0, bitlen(next_code));
    flush_pairs(outfile);

    trie_delete(root);
    close(infile);
    close(outfile);

    if (verbose) { //if v optarg is used then print stats
        double spacesave = 1.0 - ((float) total_bits / (8.0 * total_syms));
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", (uint64_t) (total_bits / 8));
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", total_syms);
        fprintf(stderr, "Space saved: %.2f%%\n", 100.0 * spacesave);
    }

    return 0;
}
