#include "batcher.h"
#include <stdio.h>
#include <stdint.h>

void comparator(Stats *stats, uint32_t *A, uint32_t x, uint32_t y) {
    if (cmp(stats, A[x], A[y]) == 1) {
        swap(stats, &A[x], &A[y]);
    }
}

void batcher_sort(Stats *stats, uint32_t *A, uint32_t n) {
    if (n == 0) {
        return;
    }

    uint32_t t = 0;

    for (uint32_t len = n; len; len >>= 1) {
        t += 1;
    }
    uint32_t p = 1 << (t - 1);
    uint32_t r, d, q;
    while (p > 0) {
        q = 1 << (t - 1);
        r = 0;
        d = p;
        while (d > 0) {
            for (uint32_t i = 0; i < (n - d); i++) {
                if ((i & p) == r) {
                    comparator(stats, A, i, i + d);
                }
            }
            d = q - p;
            q >>= 1;
            r = p;
        }
        p >>= 1;
    }
}

