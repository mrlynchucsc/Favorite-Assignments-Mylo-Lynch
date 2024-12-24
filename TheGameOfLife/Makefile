CC	= clang
CFLAGS	= -Wall -Wextra -Wpedantic -Werror -g -gdwarf-4 
TARGET	= life 
OBJS = universe.o
LFLAGS = -lncurses -lm
all: $(TARGET)

life: life.o $(OBJS)
	$(CC) $(CFLAGS) -o life life.o $(OBJS) $(LFLAGS)

life.o: life.c
	$(CC) $(CFLAGS) -c life.c

universe.o: universe.c universe.h
	$(CC) $(CFLAFS) -c universe.c

format:
	clang-format -i -style=file *.[ch]

clean:
	rm -f life *.o

