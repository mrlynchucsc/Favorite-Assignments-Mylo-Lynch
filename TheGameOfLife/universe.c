#include "universe.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

struct Universe {
    uint32_t rows; 
    uint32_t cols;
    bool **grid;
    bool toroidal; 
};

Universe *uv_create(uint32_t rows, uint32_t cols, bool toroidal){
    Universe *u = (Universe *)malloc(sizeof(Universe));
    u->rows = rows;
    u->cols = cols;
    u->toroidal = toroidal;
    
    u->grid = malloc(rows * sizeof(bool *));
    for (uint32_t r = 0; r < rows; r++) {
        u->grid[r] = calloc(cols, sizeof(bool));
    }

    return u; 
}
void uv_delete(Universe *u) {
    for (uint32_t r = 0; r < uv_rows(u); r++){
    free(u->grid[r]);}
    free(u);
    
}

uint32_t uv_cols(Universe *u) {
    return u->cols;
}

uint32_t uv_rows(Universe *u) {
    return u->rows;
}

void uv_live_cell(Universe *u, uint32_t r, uint32_t c) {
    u->grid[r][c] = true;
}

void uv_dead_cell(Universe *u, uint32_t r, uint32_t c) {
    u->grid[r][c] = false;
} 
bool uv_get_cell(Universe *u, uint32_t r, uint32_t c) {
    return u->grid[r][c];
}
bool uv_populate(Universe *u, FILE *infile) {
   uint32_t r;
   uint32_t c;
    while(fscanf(infile,"%u %u", &r, &c ) == 2) {
	if (r > u->rows || c > u->cols) {
            return false;}

        u->grid[r][c] = true; 
        }
    return true;
}

uint32_t uv_census(Universe *u, uint32_t r, uint32_t c){
    uint32_t n_neighbs = 0;

//    for (int i = -1; i <= 1; i++) {
  //      for (int j = -1; j <= 1; j++) {
    //        if (i == 0 && j == 0) {
//		    continue;
//	    }
  //      uint32_t row = (r + i + u->rows) / u->rows;
//	uint32_t col = (c + j + u->cols) % u->cols;
//
//	if (u->grid[row][col]) {
         
  //  }}}
    

    if (r > 0 && u->grid[r - 1][c]) {  // above
        n_neighbs++;
    } 
    if (c > 0 && u->grid[r][c - 1]) {   // left
        n_neighbs++;
    }
    if (r < uv_rows(u) - 1 && u->grid[r + 1][c]) {  //bottom
        n_neighbs++;
     }
    if (c < uv_cols(u) - 1 && u->grid[r][c + 1]) {  //right
        n_neighbs++;
     }
    if (c < uv_cols(u) - 1 && r < uv_rows(u) - 1 && u->grid[r + 1][c + 1]) {  //bottom right corner
        n_neighbs++;
     }
    if (c < uv_cols(u) - 1 && r > 0 && u->grid[r - 1][c + 1]) {  //top right corner 
        n_neighbs++;
     }
    if (c > 0 && r > 0 && u->grid[r - 1][c - 1]) {   //top left
        n_neighbs++;
     }
    if (c > 0 && r < uv_rows(u) - 1  && u->grid[r + 1][c - 1]) {    // bottom left
        n_neighbs++;
    }
    
    if (u->toroidal) {
        if (r == 0 && u->grid[uv_rows(u) - 1][c]) {  // above
        n_neighbs++;
    }
    if (c == 0 && u->grid[r][uv_cols(u) - 1]) {   // left
        n_neighbs++;
    }
    if (r == uv_rows(u) - 1 && u->grid[0][c]) {  //bottom
        n_neighbs++;
     }
    if (c == uv_cols(u) - 1 && u->grid[r][0]) {  //right
        n_neighbs++;
     }
    if (c == uv_cols(u) - 1 && r == uv_rows(u) - 1 && u->grid[0][0]) {  //bottom right corner
        n_neighbs++;
     }
    if (c == uv_cols(u) - 1 && r == 0 && u->grid[uv_rows(u) - 1][0]) {  //top right corner 
        n_neighbs++;
     }
    if (c == 0 && r == 0 && u->grid[uv_rows(u) - 1][uv_cols(u) - 1]) {   //top left
        n_neighbs++;
     }
    if (c == 0 && r == uv_rows(u) - 1  && u->grid[0][uv_cols(u) - 1]) {    // bottom left
        n_neighbs++;
     }
    }
            
        
    
    return n_neighbs;
}

void uv_print(Universe *u, FILE *outfile) {
   
    for (uint32_t r = 0; r < uv_rows(u); r++) {
        for (uint32_t c = 0; c < u->cols; c++) { 
	    fprintf(outfile, "%c", uv_get_cell(u, r, c) ? 'o' : '.');
	}
        fprintf(outfile, "\n");
}
    }
