#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "numtheory.h"
#include "randstate.h"
#include <time.h>
#include <unistd.h>

void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    //modular power function
    mpz_t result, base, exp;
    mpz_inits(result, base, exp, NULL);
    //initialized variables
    //take a%n
    mpz_mod(base, a, n);
    mpz_set(exp, d);
    mpz_set_ui(o, 1);

    while (mpz_cmp_ui(exp, 0) > 0) {
        if (mpz_odd_p(exp)) {
            mpz_mul(o, o, base);
            mpz_mod(o, o, n);
        }

        mpz_divexact_ui(exp, exp, 2);
        mpz_mul(base, base, base);
        mpz_mod(base, base, n);
    }
    //clear variables
    mpz_clears(result, base, exp, NULL);
}

void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    mpz_t t;
    mpz_init(t);
    //make temps in order to work with the constants
    mpz_t t_a, t_b;
    mpz_init(t_a);
    mpz_init(t_b);
    mpz_abs(t_a, a);
    mpz_abs(t_b, b);
    if (mpz_cmp(t_a, t_b) < 0) {
        mpz_set(t, t_a);
        mpz_set(t_a, t_b);
        mpz_set(t_b, t);
    }

    while (mpz_sgn(t_b) != 0) {
        mpz_mod(t, t_a, t_b);
        mpz_set(t_a, t_b);
        mpz_set(t_b, t);
    }

    mpz_set(g, t_a);

    mpz_clear(t);
    mpz_clear(t_a);
    mpz_clear(t_b);
}

bool is_prime(const mpz_t n, uint64_t iters) {
    //checks if a number is prime
    //no negatives;
    if (mpz_cmp_ui(n, 2) < 0) {
        return false;
    } else if (mpz_cmp_ui(n, 2) == 0) { //2 is prime
        return true;
    } else if (mpz_even_p(n)) {
        return false;
    }

    mpz_t r, d;
    mpz_init(r);
    mpz_init(d);
    mpz_sub_ui(d, n, 1);
    mpz_set(r, d);
    size_t s = 0;
    while (mpz_even_p(r)) {
        mpz_divexact_ui(r, r, 2);
        s++;
    }

    for (uint64_t i = 0; i < iters; i++) {
        mpz_t a;
        mpz_init(a);
        gmp_randstate_t state;
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));
        mpz_urandomm(a, state, n);
        if (mpz_cmp_ui(a, 1) <= 0 || mpz_cmp(a, d) >= 0) {
            mpz_clear(a);
            continue;
        }

        mpz_t y;
        mpz_init(y);
        pow_mod(y, a, r, n);

        bool prob_prime = false;
        if (mpz_cmp_ui(y, 1) == 0) {
            prob_prime = true;
        }
        for (size_t j = 0; j < s; j++) {
            if (mpz_cmp(y, d) == 0) {
                prob_prime = true;
                break;
            }
            mpz_t two;
            mpz_init(two);
            mpz_set_ui(two, 2);
            pow_mod(y, y, two, n);
            mpz_clear(two);
        }
        if (!prob_prime) {
            mpz_clear(a);
            mpz_clear(y);
            gmp_randclear(state);
            return false;
        }

        // Check if gcd is 1
        mpz_t g;
        mpz_init(g);
        gcd(g, a, n);
        if (mpz_cmp_ui(g, 1) != 0) {
            mpz_clear(a);
            mpz_clear(y);
            mpz_clear(g);
            gmp_randclear(state);
            return false;
        }

        //clear the variables
        mpz_clear(a);
        mpz_clear(y);
        mpz_clear(g);
        gmp_randclear(state);
    }

    //clear the rest
    mpz_clear(r);
    mpz_clear(d);

    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {

    //initailize the random state
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    while (true) {
        //random number of given size
        mpz_rrandomb(p, state, bits);

        // check bits and set left most to 1
        mpz_setbit(p, bits - 1);

        //test if prime
        if (is_prime(p, iters)) {
            printf("Generated prime %s\n", mpz_get_str(NULL, 10, p));
            break;
        }
    }
}

void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    //take the modular inverse of a in respect to n, outcome is put in o
    mpz_t r, r0, t, t0, q, tmp, g;
    mpz_inits(r, r0, t, t0, q, tmp, g, NULL);
    //find gcd
    gcd(g, a, n);
    if (mpz_cmp_ui(g, 1) != 0) {
        mpz_set_ui(o, 0);
        return;
    }
    //psudocode from numtheory section of asgn
    mpz_set(r, n);
    mpz_set(r0, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t0, 1);
    while (mpz_cmp_ui(r0, 0) != 0) {
        mpz_fdiv_q(q, r, r0);
        mpz_set(tmp, r0);
        mpz_mul(r0, q, r0);
        mpz_sub(r0, r, r0);
        mpz_set(r, tmp);
        mpz_set(tmp, t0);
        mpz_mul(t0, q, t0);
        mpz_sub(t0, t, t0);
        mpz_set(t, tmp);
    }

    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(o, 0);
        return;
    }
    if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(t, t, n);
    }

    mpz_set(o, t);

    mpz_clears(r, r0, t, t0, q, tmp, g, NULL);
}
