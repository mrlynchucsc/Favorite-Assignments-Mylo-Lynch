#include "ss.h"
#include "randstate.h"
#include <stdlib.h>
#include "numtheory.h"
#include <string.h>

void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    //# of bits for p & q
    uint64_t pits = (random() % (nbits / 5)) + (nbits / 5);
    uint64_t qits = nbits - 2 * pits;

    //get p and q primes
    make_prime(q, qits, iters);
    make_prime(p, pits, iters);

    //n = p^2 * q cant use pow so multiply p by p
    mpz_mul(n, p, p);
    mpz_mul(n, n, q);
}

void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    //n needs to be a hex string
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, username);
}

void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    // Read hexstring that was made in write
    char hexstr[512];
    if (fgets(hexstr, 512, pbfile) == NULL) {
        fprintf(stderr, "Error reading public key file: n has no value\n");
        exit(1);
    }
    //set value to variable n
    mpz_set_str(n, hexstr, 16);

    //Read username
    if (fgets(username, 512, pbfile) == NULL) {
        fprintf(stderr, "Error reading public key file: username has no value\n");
        exit(1);
    }

    // clean up extra new line from username
    size_t size = strlen(username);
    if (size > 0 && username[size - 1] == '\n') {
        username[size - 1] = '\0';
    }
}

void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {
    //create variables to be populated with values to find the lcm
    mpz_t p_sub_1, q_sub_1, n, lambda, gcd_divisor;
    mpz_inits(p_sub_1, q_sub_1, n, lambda, gcd_divisor, NULL);
    //multiply p and q so the parameter actually has a value
    mpz_mul(pq, p, q);

    // n = p^2 * q like in previous functions
    mpz_mul(n, p, p);
    mpz_mul(n, n, q);

    // p - 1 and q - 1
    mpz_sub_ui(p_sub_1, p, 1);
    mpz_sub_ui(q_sub_1, q, 1);

    // (p - 1) * (q - 1) / gcd(p - 1, q - 1) = lcm, formula from Omar Ahmadyar's wednesday section
    gcd(gcd_divisor, p_sub_1, q_sub_1);
    mpz_div(lambda, p_sub_1, gcd_divisor);
    mpz_mul(lambda, lambda, q_sub_1);
    // get d
    mod_inverse(d, n, lambda);

    // Free variables that were initialized within the function to prevent memory leaks
    mpz_clears(q_sub_1, p_sub_1, n, lambda, gcd_divisor, NULL);
}
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
}
void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
}

void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    //make into hex like write_pub
    char *pq_str = mpz_get_str(NULL, 16, pq);
    char *d_str = mpz_get_str(NULL, 16, d);

    //Print to file
    fprintf(pvfile, "%s\n%s\n", pq_str, d_str);

    free(pq_str);
    free(d_str);
}

void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    char pq_str[1024 + 1];
    char d_str[1024 + 1];

    // get pq and turn it into mpz_t like every other variable this code works with
    if (fgets(pq_str, 1024 + 1, pvfile) == NULL) {
        fprintf(stderr, "Error: failed to read pq\n");
        exit(EXIT_FAILURE);
    }
    if (mpz_set_str(pq, pq_str, 16) != 0) {
        fprintf(stderr, "Error: failed to convert pq\n");
        exit(EXIT_FAILURE);
    }

    // get d and turn it into mpz_t like every other variable this code works with
    if (fgets(d_str, 1024 + 1, pvfile) == NULL) {
        fprintf(stderr, "Error: failed to read d\n");
        exit(EXIT_FAILURE);
    }
    if (mpz_set_str(d, d_str, 16) != 0) {
        fprintf(stderr, "Error: failed to convert d\n");
        exit(EXIT_FAILURE);
    }
}

void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {
    // find the size of the block to be able to allocate the proper amount of memory for the block
    int block_size = (mpz_sizeinbase(n, 2));
    block_size = block_size / 2;
    block_size = block_size - 2;
    block_size = block_size / 8;

    //use the calculated size to allocate memory
    uint8_t *block = (uint8_t *) malloc(block_size);

    //0xFF should be the value of the first block byte
    block[0] = 0xFF;

    while (!feof(infile)) {
        //only able to read block_size - 1 max
        int read = fread(&block[1], 1, block_size - 1, infile);

        //if less, then the file is done
        if (read < block_size - 1) {
            //bytes left over padded with zeros
            memset(&block[read + 1], 0, block_size - read - 1);

            // first byte set to the read
            block[0] = (uint8_t) read;
        }

        // Change the block to mpz_t like all other variables to work with
        mpz_t message;
        mpz_init(message);
        mpz_import(message, block_size, 1, sizeof(uint8_t), 1, 0, block);

        //now that we can work with it, check if the message satisfies condition
        mpz_t zero, one;
        mpz_init_set_ui(zero, 0);
        mpz_init_set_ui(one, 1);
        while (mpz_cmp(message, zero) == 0 || mpz_cmp(message, one) == 0) {
            //increment by one until condition is not met and exit while loop
            mpz_add_ui(message, message, 1);
        }

        //encrypt the message
        mpz_t crypt;
        mpz_init(crypt);
        ss_encrypt(crypt, message, n);

        //outfile needs crypt
        char *crypt_str = mpz_get_str(NULL, 16, crypt);
        fprintf(outfile, "%s\n", crypt_str);
        free(crypt_str);

        mpz_clear(message);
        mpz_clear(crypt);
    }

    free(block);
}

void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {
    //find the size of the block to be able to allocate the proper amount of memory for the block
    const int block_size = (mpz_sizeinbase(pq, 2) - 1) / 8;
    uint8_t *block = (uint8_t *) calloc(block_size, sizeof(uint8_t));

    // go through all lines within the infile
    char line_size[1024];
    mpz_t crypt;
    mpz_init(crypt);
    while (fgets(line_size, sizeof(line_size), infile)) {
        // Scan hexstring and convert to mpz_t
        if (mpz_set_str(crypt, line_size, 16) == -1) {
            printf("Error: failed to read hexstring.\n");
            exit(1);
        }

        //initialize message and set value to decrypted crypt
        mpz_t message;
        mpz_init(message);
        ss_decrypt(message, crypt, d, pq);

        //convert m into bytes by using mpz_export and put them in the block
        size_t read;
        mpz_export(block + 1, &read, 1, 1, 1, 0, message);

        //once converted into bytes we're able to finally write out our actual message
        fwrite(block + 2, sizeof(uint8_t), read - 1, outfile);

        //clear variables

        mpz_clear(message);
    }

    mpz_clear(crypt);
    //Prevent memory leaks
    free(block);
}
