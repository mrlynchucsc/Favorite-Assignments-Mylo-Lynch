#include "universe.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>

#define OPTIONS "tsn:i:o:h"
#define DELAY 50000

int main(int argc, char **argv) {
    int opt = 0;
    bool toroidal, s = false;
    uint32_t r, c;
    int generations = 100;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't': toroidal = true; break;
        case 's': s = true; break;
        case 'n': generations = atoi(optarg); break;
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
	case 'h': printf("SYNOPSIS\n""The Game of Life\n""USAGE\n""./life [-tsn:i:o:h] [-n number] [-i file] [-o file]\n\n""OPTIONS\n""-h Display program help and usage.\n""-t Create your universe as a toroidal\n"" -s Silent - do not use animate the evolution\n""-n {number}    Number of generations [default: 100]\n""-i {file} Input file [default: stdin]\n""-o {file}      Output file [default: stdout]\n");
        break;
	}
    }
  

    fscanf(infile,"%u %u", &r, &c);;
    Universe *uA = uv_create(r, c, toroidal);
    Universe *uB = uv_create(r, c, toroidal);

    uv_populate(uA, infile);
    if (uv_populate(uA, infile) == false) {
        perror("Failed to populate the universe. Invalid row/column range");
        return 0;
    }
    for (int gen = 0; gen < generations; gen++) {
        for (uint32_t column = 0; column < uv_cols(uA); column++) {
            for (uint32_t row = 0; row < uv_rows(uA); row++) {
		if (uv_census(uA, row, column) == 3 && uv_get_cell(uA, row, column) == false){
                    uv_live_cell(uB, row, column);
	       }
                else if ((uv_get_cell(uA, row, column) == true) && (uv_census(uA, row, column) == 2 || uv_census(uA, row, column) == 3)) {
                    uv_live_cell(uB, row, column);
		}
		else {
                    uv_dead_cell(uB, row, column);
               }
	    }
	}
    Universe *temp = uA;
    uA = uB;
    uB = temp;
    if (!s) {
        initscr();
	clear();
        curs_set(FALSE);
        for (uint32_t row = 0; row <uv_rows(uA); row++) {
        for (uint32_t col = 0; col < uv_cols(uA); col++) {
            if (uv_get_cell(uA, row, col)) {
                mvprintw(row, col, "o");
            }
	    else{
                mvprintw(row, col, ".");
	    }
	}
	}
        refresh();
        usleep(DELAY);
    }
    }
    endwin();
    //print final
    uv_print(uA, outfile);

    //close files

    //Clean up
    uv_delete(uA);
    uv_delete(uB);

    fclose(infile);
    fclose(outfile);


    return 0;
    }    
