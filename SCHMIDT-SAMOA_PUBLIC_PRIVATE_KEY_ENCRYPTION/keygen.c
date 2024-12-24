#include <unistd.h>
#include "ss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ss.h"
#include "randstate.h"
#include <time.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    int option;
    uint64_t bits = 256;
    uint64_t iters = 50;
    const char *file_pubkey = "ss.pub";
    const char *file_privkey = "ss.priv";
    uint64_t seed = time(NULL);
    bool verbose = false;

    while ((option = getopt(argc, argv, "b:i:n:d:s:vh")) != -1) {
        switch (option) {
        case 'b': bits = atoi(optarg); break;
        case 'i': iters = atoi(optarg); break;
        case 'n': file_pubkey = optarg; break;
        case 'd': file_privkey = optarg; break;
        case 's': seed = atoi(optarg); break;
        case 'v': verbose = true; break;
        case 'h':
            printf("SYNOPSIS:\n   Generates an SS public/private key pair.\n\nUSAGE\n   ./keygen "
                   "[OPTIONS]\n\nOPTIONS\n  -h\t\tDisplay program help and usage.\n  -v\t\tDisplay "
                   "verbose program output.\n  -b bits\tMinimum bits needed for public key n "
                   "(default: 256).\n  -i iterations\tMiller-Rabin iterations for testing primes "
                   "(default: 50).\n  -n pbfile\tPublic key file (default: ss.pub).\n  -d "
                   "pvfile\tPrivate key file (default: ss.priv).\n  -s seed\tRandom seed for "
                   "testing.\n");
            break;
        default:
            printf("SYNOPSIS:\n   Generates an SS public/private key pair.\n\nUSAGE\n   ./keygen "
                   "[OPTIONS]\n\nOPTIONS\n  -h\t\tDisplay program help and usage.\n  -v\t\tDisplay "
                   "verbose program output.\n  -b bits\tMinimum bits needed for public key n "
                   "(default: 256).\n  -i iterations\tMiller-Rabin iterations for testing primes "
                   "(default: 50).\n  -n pbfile\tPublic key file (default: ss.pub).\n  -d "
                   "pvfile\tPrivate key file (default: ss.priv).\n  -s seed\tRandom seed for "
                   "testing.\n");
            exit(EXIT_FAILURE);
        }
    }
    //get username
    const char *username = getenv("USER");

    //initialize the random state
    randstate_init(seed);

    //make public key
    mpz_t p, q, n;
    mpz_inits(p, q, n, NULL);

    ss_make_pub(p, q, n, bits, iters);

    //add public key to the file
    FILE *pbfile = fopen(file_pubkey, "w");
    if (!pbfile) {
        fprintf(stderr, "Error: could not open public key file %s\n", file_pubkey);
        exit(EXIT_FAILURE);
    }
    ss_write_pub(n, username, pbfile);
    fclose(pbfile);

    //make private key
    mpz_t d, pq;
    mpz_inits(d, pq, NULL);

    ss_make_priv(d, pq, p, q);

    //add the private key to the file
    FILE *pvfile = fopen(file_privkey, "w");
    if (!pvfile) {
        fprintf(stderr, "Error: could not open private key file %s\n", file_privkey);
        exit(EXIT_FAILURE);
    }

    ss_write_priv(pq, d, pvfile);

    //setthe file permissions to 0600
    int fd = fileno(pvfile);
    if (fd == -1) {
        fprintf(stderr, "Error: could not get file descriptor for %s\n", file_privkey);
        exit(EXIT_FAILURE);
    }
    if (fchmod(fd, S_IRUSR | S_IWUSR) == -1) {
        fprintf(stderr, "Error: could not set file permissions for %s\n", file_privkey);
        exit(EXIT_FAILURE);
    }

    fclose(pvfile);

    if (verbose) {
        printf("user = %s\n", username);
        gmp_printf("p  (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q  (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n  (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    //clear initialized variables
    mpz_clears(p, q, n, d, pq, NULL);
    randstate_clear();

    return 0;
}
