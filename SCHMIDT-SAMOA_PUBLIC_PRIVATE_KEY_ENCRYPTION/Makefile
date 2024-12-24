CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic $(shell pkg-config --cflags gmp)
LFLAGS = $(shell pkg-config --libs gmp)
TARGETS = keygen encrypt decrypt
OBJS = randstate.o numtheory.o ss.o keygen.o encrypt.o decrypt.o

all: keygen encrypt decrypt

keygen: keygen.o numtheory.o ss.o randstate.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

encrypt: encrypt.o ss.o numtheory.o randstate.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

decrypt: decrypt.o ss.o numtheory.o randstate.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

randstate.o: randstate.c
	$(CC) $(CFLAGS) -c $<

numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c $<

ss.o: ss.c
	$(CC) $(CFLAGS) -c $<

keygen.o: keygen.c
	$(CC) $(CFLAGS) -c $<

encrypt.o: encrypt.c
	$(CC) $(CFLAGS) -c $<

decrypt.o: decrypt.c
	$(CC) $(CFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGETS) $(OBJS) simon.o

format:
	clang-format -i -style=file *.[ch]
