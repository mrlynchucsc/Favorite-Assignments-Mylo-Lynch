##SCHMIDT-SAMOA PUBLIC/PRIVATE KEY ENCRYPTION AND DECRYPTION
this program will encrypt and decrypt the given text files as well, as well as generate public and private keys to do so.

##How to build and execute
```make``` or ```make keygen``` ```make encrypt``` ```make decrypt``` to build the individual executables. 
To execute, use ```./``` and the desired executable name (one of the three in the above "make" statements). To print the usage and find out how to use these executables, run them with ```-h``` to print the usage. Here's an example usage from encrypt.c to know what to expect: ```"SYNOPSIS:\n   Encrypts data using an SS encryption.\n   Encrypted data is decrypted by the decrypt program.\n\nUSAGE\n   ./encrypt [OPTIONS]\n\nOPTIONS\n  -h\t\t\tDisplay program help and usage.\n  -v\t\t\tDisplay verbose program output.\n  -i infile\t\tInput file of data to encrypt (default: stdin).\n  -o outfile\tOutput file for encrypted data (default: stdout).\n  -n pbfile\t\tPublic key file (default: ss.pub).\n"```

##How to clean and format
```make clean``` to clean all objects and executables
```make format``` to format

