CC = clang
CFLAGS = -Wall -Wextra -Werror

all: encode decode

encode: io.o word.o trie.o encode.o
	$(CC) $(CFLAGS) -o encode io.o word.o trie.o encode.o

decode: io.o word.o trie.o decode.o
	$(CC) $(CFLAGS) -o decode io.o word.o trie.o decode.o

io.o: io.c io.h code.h endian.h
	$(CC) $(CFLAGS) -c io.c

word.o: word.c word.h code.h endian.h
	$(CC) $(CFLAGS) -c word.c

trie.o: trie.c trie.h code.h endian.h
	$(CC) $(CFLAGS) -c trie.c

encode.o: encode.c io.h word.h trie.h
	$(CC) $(CFLAGS) -c encode.c

decode.o: decode.c io.h word.h trie.h
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f encode decode *.o

format:
	clang-format -i -style=file *.[ch]

