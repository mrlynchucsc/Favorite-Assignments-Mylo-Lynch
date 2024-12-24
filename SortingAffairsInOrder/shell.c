#include "shell.h"

#include "gaps.h"
#include "stats.h"

#include <stdint.h>
#include <stdlib.h>

void shell_sort(Stats *stats, uint32_t *arr, uint32_t length) {

    for (uint32_t i = 0; i < GAPS; i++) {
        uint32_t j = gaps[i];

        for (uint32_t k = j; k < length; k++) {
            uint32_t x = k;
            uint32_t temp = move(stats, arr[k]);
            while ((x >= j) && cmp(stats, arr[k],  arr[x - j]) < 0) {
                arr[x] = move(stats, arr[x - j]);
                x -= j;
            }

            arr[x] = move(stats, temp);
        }
    }
}

