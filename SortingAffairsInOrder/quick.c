#include "quick.h"

#include "stats.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t partition(Stats *stats, uint32_t *A, uint32_t low, uint32_t high) {
    uint32_t i = low - 1;
    for (uint32_t j = low; cmp(stats, high, j) >= 0; j++) {
        if (A[j - 1] < A[high - 1]) {
            //printf("%dj - one \n", A[j - 1]);

            i++;
          //  printf("%d i \n", A[i - 1]);
            swap(stats, &A[i - 1], &A[j - 1]);
        }
        //printf("%d index \n", i);
      //  printf("%d A[i] before \n", A[i]);
    }
    swap(stats, &A[i], &A[high - 1]);
  //  printf("%d partition output \n", i + 1 );
    return i + 1;
}

void quick_sorter(Stats *stats, uint32_t *A, uint32_t low, uint32_t high) {
    uint32_t p;
    if (low < high) {
        p = partition(stats, A, low, high);
        quick_sorter(stats, A, low, p - 1);
	quick_sorter(stats, A, p + 1, high);
    }
}

void quick_sort(Stats *stats, uint32_t *A, uint32_t n) {
    quick_sorter(stats, A, 1, n);
}


