#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <unistd.h>
#include "ss.h"

int main(int argc, char *argv[]) {
    char *input_filename = NULL;
    char *output_filename = NULL;
    char *privkey_filename = "ss.priv";
    bool verbose = false;
    int options;

    //command line switch statements once more
    while ((options = getopt(argc, argv, "i:o:n:vh")) != -1) {
        switch (options) {
        case 'i': input_filename = optarg; break;
        case 'o': output_filename = optarg; break;
        case 'n': privkey_filename = optarg; break;
        case 'v': verbose = true; break;
        case 'h':
            printf("SYNOPSIS:\n   Encrypts data using an SS encryption.\n   Encrypted data is "
                   "decrypted by the decrypt program.\n\nUSAGE\n   ./encrypt "
                   "[OPTIONS]\n\nOPTIONS\n  -h\t\t\tDisplay program help and usage.\n  "
                   "-v\t\t\tDisplay verbose program output.\n  -i infile\t\tInput file of data to "
                   "encrypt (default: stdin).\n  -o outfile\tOutput file for encrypted data "
                   "(default: stdout).\n  -n pbfile\t\tPublic key file (default: ss.pub).\n");
            break;
        default:
            printf("SYNOPSIS:\n   Encrypts data using an SS encryption.\n   Encrypted data is "
                   "decrypted by the decrypt program.\n\nUSAGE\n   ./encrypt "
                   "[OPTIONS]\n\nOPTIONS\n  -h\t\t\tDisplay program help and usage.\n  "
                   "-v\t\t\tDisplay verbose program output.\n  -i infile\t\tInput file of data to "
                   "encrypt (default: stdin).\n  -o outfile\tOutput file for encrypted data "
                   "(default: stdout).\n  -n pbfile\t\tPublic key file (default: ss.pub).\n");
            return 0;
        }
    }

    //Open privkey file for usage
    FILE *privkey_file = fopen(privkey_filename, "r");
    if (privkey_file == NULL) {
        perror("Error opening public key file");
        exit(1);
    }

    //Read pq and d
    mpz_t pq, d;
    mpz_inits(pq, d, NULL);
    ss_read_priv(pq, d, privkey_file);

    //if verbose print the verbose output
    if (verbose) {
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    //decryption magic
    //default input is stdin
    FILE *input_file = stdin;
    if (input_filename != NULL) {
        input_file = fopen(input_filename, "r");
    }
    //default output is stdout
    FILE *output_file = stdout;
    if (output_filename != NULL) {
        output_file = fopen(output_filename, "w");
    }

    ss_decrypt_file(input_file, output_file, d, pq);

    //Be closed, clear, and free!!
    if (input_file != stdin) {
        fclose(input_file);
    }
    if (output_file != stdout) {
        fclose(output_file);
    }
    fclose(privkey_file);
    mpz_clear(pq);
    mpz_clear(d);

    return 0;
}
