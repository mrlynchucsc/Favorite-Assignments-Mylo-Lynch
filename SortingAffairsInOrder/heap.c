#include "heap.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "heap.h"

uint32_t max_child(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    uint32_t left = 2 * first;
    uint32_t right = left + 1;
    if (right <= last && cmp(stats, A[right - 1], A[left - 1]) == 1) {
//        printf("%d right \n", right);
        return right;
    } else {
  //      printf("%d left \n", left);
        return left;
    }
}

void fix_heap(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    uint32_t found = 0;
    uint32_t mother = first;
    uint32_t great = max_child(stats, A, mother, last);

    while (mother <= last / 2 && found == 0) {
  //      printf("%d mother \n", A[mother-1]);
//	printf("%d great \n", A[great-1]);
        if (A[mother - 1] < A[great - 1]) {
            swap(stats, &A[mother - 1], &A[great - 1]);
            mother = move(stats, great);
            great = move(stats, max_child(stats, A, mother, last));
        } else {
            found = 1;
        }
    }
}

void build_heap(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    for (uint32_t father = (last - 1) / 2; father > first - 1; father--) {
       // printf("%d \n", father);
        fix_heap(stats, A, father, last);
    }
}

void heap_sort(Stats *stats, uint32_t *A, uint32_t n) {
    uint32_t first = 1;
    uint32_t last = n;
    build_heap(stats, A, first, last);
    for (uint32_t leaf = last; leaf > first; leaf--) {
        swap(stats, &A[first - 1], &A[leaf - 1]);
        fix_heap(stats, A, first, leaf - 1);
    }
}

