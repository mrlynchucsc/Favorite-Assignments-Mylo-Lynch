##ASGN 4 THE GAME OF LIFE

##short description
Conway's game of life in c

##How to compile
`make all` or `make life`

##How to clean & format
`make clean` and `make format`

##Files with Code
life.c and universe.c header for universe is universe.h
##Bugs
2 memory leaks from uninstantiated values caused by life.c.main.4

##Makefile
`CC      = clang
CFLAGS  = -Wall -Wextra -Wpedantic -Werror -g -gdwarf-4
TARGET  = life
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
`

