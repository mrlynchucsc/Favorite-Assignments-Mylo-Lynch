# Assignment 1 directory

This directory contains source code and other files for Assignment 1.

# Memory Program

## Overview
The `memory` program is a command-line utility written in C that provides a get/set memory abstraction for files in a Linux directory. It allows users to read (get) the contents of a file or write (set) content to a file within the current working directory.

## Usage
./memory

get
{existing filename}

and then the contents of that file will be printed to stdout

./memory

set
{filename}
{size}
{content}

then OK will be printed to STDOUT indicating that the file has been written with the content that was set.
### Compile the Program
To compile the `memory` program, ensure you are in the project directory and run the following command:

```bash
make all
```
```
make clean
```
to clean it.
