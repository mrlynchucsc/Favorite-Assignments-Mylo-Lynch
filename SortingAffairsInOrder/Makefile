CC	= clang
CFLAGS	= -Wall -Wextra -Wpedantic -Werror -g
TARGET	= sorting 

all: $(TARGET)

sorting: sorting.c heap.c batcher.c quick.c shell.c set.c stats.c 
	$(CC)	-o sorting heap.c batcher.c quick.c shell.c set.c stats.c sorting.c -lm

sorting.o: *.c
	$(CC)	$(CFLAGS) -c *.c

format:
	clang-format -i -style=file *.[ch]

clean:
	rm -f sorting *.o

