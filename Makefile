CC = gcc
CFLAGS = -Wall -Wextra -pedantic -I./src -g
SRC = src/main.c src/routes.c src/request.c
OBJ = $(SRC:.c=.o)
TARGET = bin/fastserver

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean