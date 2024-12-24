CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic
TARGET = memory
SRC = memory.c
FORMAT = clang-format

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean format

format:
	$(FORMAT) -i $(SRC)

