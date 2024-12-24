#include "io.h"

#include "code.h"
#include "endian.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BITS 8 // there are 8 bits in a byte

uint64_t total_bits = 0;
uint64_t total_syms = 0;

static uint8_t sym_buffer[BLOCK] = { 0 };
static int sym_index = 0;

static uint8_t bit_buffer[BLOCK] = { 0 };
static int bit_index = 0;

// used to buffer BLOCK of data
//basically a wrapper for syscall read()
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int reads = -1, total = 0;

    while ((total != to_read) && (reads != 0)) {
        if ((reads = read(infile, buf, to_read)) == -1) {
            fprintf(stderr, "failed to read\n");
            exit(EXIT_FAILURE);
        }

        total += reads;
        buf += reads;
    }

    return total;
}

// writes buffered blocks to output destination file
//
//wrapper for write
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int writes = -1, total = 0;

    while ((total != to_write) && (writes != 0)) {
        if ((writes = write(outfile, buf, to_write)) == -1) {
            fprintf(stderr, "failed to write bytes to file.\n");
            exit(EXIT_FAILURE);
        }

        total += writes;
        buf += writes;
    }

    return total;
}

// reads header n checks for endianness
//swaps bits depending on endianness
void read_header(int infile, FileHeader *header) {
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));

    if (big_endian()) { //if its big endian then it swaps the bits
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    total_bits += (sizeof(FileHeader) * BITS);
}

//same as read.. except it writes
// still checks for endianness
void write_header(int outfile, FileHeader *header) {
    if (big_endian()) {
        header->magic = swap32(header->magic); //swaps header if big endian
        header->protection = swap16(header->protection);
    }

    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    total_bits += (sizeof(FileHeader) * BITS); //account for total bits
}

//buffers symbols
//
bool read_sym(int infile, uint8_t *sym) {
    static int end = -1;

    if (!sym_index) { //if not sym_index
        int reads = read_bytes(infile, sym_buffer, BLOCK);
        if (reads < BLOCK) {
            end = reads + 1;
        }
    }

    *sym = sym_buffer[sym_index];
    sym_index = (sym_index + 1) % BLOCK;

    if (sym_index != end) {
        total_syms += 1;
    }

    return sym_index == end ? false : true;
}

//write sym and code pairs for lz78
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    if (big_endian()) {
        swap16(code);
    }

    for (int bit = 0; bit < bitlen; bit++) {
        if (bit_index == BITS * BLOCK) {
            write_bytes(outfile, bit_buffer, BLOCK);
            memset(bit_buffer, 0, BLOCK);
            bit_index = 0;
        }

        if ((code >> (bit % 8)) & 1) {
            bit_buffer[bit_index / 8] |= (1 << (bit_index % 8));
        }

        bit_index += 1;
    }

    for (int bit = 0; bit < BITS; bit++) {
        if (bit_index == BITS * BLOCK) { //if the index is equal to the full amount of bits write
            write_bytes(outfile, bit_buffer, BLOCK);
            memset(bit_buffer, 0, BLOCK); //set memory
            bit_index = 0; //reset index
        }

        if ((sym >> (bit % 8)) & 1) {
            bit_buffer[bit_index / 8] |= (1 << (bit_index % 8));
        }

        bit_index += 1; //iterate through by adding to index
    }

    total_bits += (bitlen + BITS);
}

//flush whats left in the toilet (the block)
void flush_pairs(int outfile) {
    int write_var = bit_index / 8;
    write_bytes(outfile, bit_buffer, write_var);
}

// buffers and reads pairs from input file
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0, *sym = 0;

    for (int bit = 0; bit < bitlen; bit++) {
        if (!bit_index) {
            read_bytes(infile, bit_buffer, BLOCK);
        }

        *code |= ((bit_buffer[bit_index / 8] >> (bit_index % 8)) & 1) << bit;
        bit_index = (bit_index + 1) % (BITS * BLOCK);
    }

    for (int bit = 0; bit < BITS; bit++) {
        if (!bit_index) {
            read_bytes(infile, bit_buffer, BLOCK);
        }

        *sym |= ((bit_buffer[bit_index / 8] >> (bit_index % 8)) & 1) << bit;
        bit_index = (bit_index + 1) % (BITS * BLOCK);
    }

    if (big_endian()) {
        *code = swap16(*code);
    }

    total_bits += (bitlen + BITS);

    return *code != STOP_CODE ? true : false;
}

//buff words n syms and write into output
void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        if (sym_index == BLOCK) {
            write_bytes(outfile, sym_buffer, BLOCK);
            sym_index = 0;
        }

        sym_buffer[sym_index] = w->syms[i];
        sym_index += 1;
    }

    total_syms += w->len;
}

// flush whats left inside the buffer
void flush_words(int outfile) {
    write_bytes(outfile, sym_buffer, sym_index);
}
