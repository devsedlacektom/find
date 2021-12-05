CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -std=c99 -Wall -Wextra -pedantic -O3
DEPS = arguments.h find.h userStructures.h
OBJ = arguments.o find.o main.o userStructures.o

.DEFAULT_GOAL = all
.PHONY = all clean remove

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

all: find clean

find: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ)

remove: clean
	rm -f find