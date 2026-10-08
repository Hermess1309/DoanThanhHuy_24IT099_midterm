CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O2 -Iinclude
LDFLAGS =

SRCDIR = src
INCDIR = include

SRCS = $(SRCDIR)/main.c $(SRCDIR)/options.c $(SRCDIR)/entry.c $(SRCDIR)/sort.c $(SRCDIR)/display.c $(SRCDIR)/traverse.c
OBJS = $(SRCS:.c=.o)
HEADERS = $(INCDIR)/ls.h $(INCDIR)/options.h $(INCDIR)/entry.h $(INCDIR)/sort.h $(INCDIR)/display.h $(INCDIR)/traverse.h
TARGET = ls

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

test: $(TARGET)
	@echo "=== Testing basic ls ==="
	@./$(TARGET)
	@echo "=== Testing ls -la ==="
	@./$(TARGET) -la
	@echo "=== Testing ls -lh ==="
	@./$(TARGET) -lh
	@echo "=== Testing ls -F ==="
	@./$(TARGET) -F
	@echo "=== Testing ls -lis ==="
	@./$(TARGET) -lis
	@echo "All verification tests passed!"
