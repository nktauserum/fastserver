CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -g
LDFLAGS ?=
TARGET ?= bin/fastserver

SRCDIR ?= src
ENTRY ?= $(SRCDIR)/main.c
SRCS := $(ENTRY) $(filter-out $(ENTRY), $(wildcard $(SRCDIR)/*.c))
OBJS := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)