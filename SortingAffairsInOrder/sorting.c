#include "set.h"
#include "stats.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OPTIONS "asbqhrnpH"
#define PRIu32  "u"

void print_array(uint32_t *array, int max_print) {
    for (int i = 0; i < max_print; ++i) {
        printf("%13" PRIu32 "", array[i]);
        if (i % 5 == 4) {
            printf("\n");
        }
    }
}
int main(int argc, char **argv) {
    int opt = 0;
    uint64_t seed = 13371453;
    int size = 100;
    int elements = 100;
    Set op = 0;
    Stats stats;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'a': op = set_union(op, 0xF); break;
        case 's':
            //shell
            op = set_insert(op, 0);
            break;
        case 'b': op = set_insert(op, 1); break; //batcher
        case 'q':
            op = set_insert(op, 2);
            //quick
            break;
        case 'h':
            op = set_insert(op, 3);
            // heap
            break;
        case 'r': seed = rand(); break;
        case 'n': size = atoi(optarg);  break;
        case 'p': elements = strtol(optarg, NULL, 10); break;
        case 'H': printf("SYNOPSIS\n"
                   "   A collection of comparison-based sorting algorithms.\n"
                   "USAGE\n"
                   "./sorting [-Hasbhqn:p:r:] [-n length] [-p elements] [-r seed]\n\n"
                   "OPTIONS\n"
                   "   -H              Display program help and usage.\n"
                   "   -a              Enable all sorts.\n"
                   "   -s              Enable Shell Sort.\n"
                   "   -b              Enable Batcher Sort.\n"
                   "   -h              Enable Heap Sort.\n"
                   "   -q              Enable Quick Sort.\n"
                   "   -n length       Specify number of array elements.\n"
                   "   -p elements     Specify number of elements to print.\n"
                   "   -r seed         Specify random seed (default: 13371453).\n");
            break;
	default: return -1;
        }
    }
   
    uint32_t empty_array[size];
    for (int i = 0; i < size; i++) {
        empty_array[i] = rand() & 0x3fffffff;
    }

    int maxp = size >= elements ? elements : size;

    for (int i = 0; i < 4; i++) {
        if (set_member(op, i)) {
            if (i == 0) {
                reset(&stats);
                uint32_t *shell_array = calloc(size, sizeof(uint32_t));
                memcpy(shell_array, empty_array, sizeof(uint32_t) * size);
                shell_sort(&stats, shell_array, size);
                printf("Shell Sort, %d elements, %llu moves, %llu compares\n",size,stats.moves,stats.compares);
		print_array(shell_array, maxp);
                free(shell_array);
            }
            if (i == 1) {
                reset(&stats);
                uint32_t *batcher_array = calloc(size, sizeof(uint32_t));
                memcpy(batcher_array, empty_array, sizeof(uint32_t) * size);
                batcher_sort(&stats, batcher_array, size);
                printf("Batcher Sort, %d elements, %llu moves, %llu compares\n",size,stats.moves,stats.compares);
                print_array(batcher_array, maxp);
                free(batcher_array);
            }
            if (i == 2) {
                reset(&stats);
                uint32_t *quick_array = calloc(size, sizeof(uint32_t));
                memcpy(quick_array, empty_array, sizeof(uint32_t) * size);
                quick_sort(&stats, quick_array, size);
                printf("Quick Sort, %d elements, %llu moves, %llu compares\n",size,stats.moves,stats.compares);
                print_array(quick_array, maxp);
                free(quick_array);
            }
            if (i == 3) {
                printf("Running heap sort\n");
                uint32_t *heap_array = calloc(size, sizeof(uint32_t));
                memcpy(heap_array, empty_array, sizeof(uint32_t) * size);
		//print_array(heap_array, maxp);
                heap_sort(&stats, heap_array, size);
                printf("Heap Sort, %d elements, %llu moves, %llu compares\n",size,stats.moves,stats.compares);
                print_array(heap_array, maxp);
                free(heap_array);
            }
        }
    }
}
