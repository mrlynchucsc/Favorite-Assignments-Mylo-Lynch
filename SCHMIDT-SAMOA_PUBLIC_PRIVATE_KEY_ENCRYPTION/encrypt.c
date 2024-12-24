#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <unistd.h>
#include "ss.h"

int main(int argc, char *argv[]) {
    char *input_filename = NULL;
    char *output_filename = NULL;
    char *pubkey_filename = "ss.pub";
    bool verbose = false;
    int options;

    //Switch statement like almost all the past assignments
    while ((options = getopt(argc, argv, "i:o:n:vh")) != -1) {
        switch (options) {
        case 'i': input_filename = optarg; break;
        case 'o': output_filename = optarg; break;
        case 'n': pubkey_filename = optarg; break;
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

    //open the public file
    FILE *pubkey_file = fopen(pubkey_filename, "r");

    // Read public key (n) and username
    mpz_t n;
    mpz_init(n);
    char username[1024];
    ss_read_pub(n, username, pubkey_file);

    //if -v is used in the command line print the verbose output
    if (verbose) {
        gmp_printf("user = %s\n", username);
        gmp_printf("Public key (n) (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
    }

    //where the encryption magic happens
    //default input is stdin
    FILE *input_file = stdin;
    if (input_filename != NULL) {
        input_file = fopen(input_filename, "r");
    }
    // default output is stdout
    FILE *output_file = stdout;
    if (output_filename != NULL) {
        output_file = fopen(output_filename, "w");
    }

    ss_encrypt_file(input_file, output_file, n);

    //clear variables and close files
    if (input_file != stdin) {
        fclose(input_file);
    }
    if (output_file != stdout) {
        fclose(output_file);
    }
    fclose(pubkey_file);
    mpz_clear(n);

    return 0;
}
