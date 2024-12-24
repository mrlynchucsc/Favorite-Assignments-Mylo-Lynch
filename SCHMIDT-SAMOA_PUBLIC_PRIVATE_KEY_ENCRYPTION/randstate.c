#include "randstate.h"
#include <stdint.h>
#include <gmp.h>

gmp_randstate_t state;

//
// Initializes the random state needed for SS key generation operations.
// Must be called before any key generation or number theory operations are used.
//
// seed: the seed to seed the random state with.
//

void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

//
// Frees any memory used by the initialized random state.
// Must be called after all key generation or number theory operations are used.
//

void randstate_clear(void) {
    gmp_randclear(state);
}
