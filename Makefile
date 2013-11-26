CC=gcc
CFLAGS=-pthread
LDFLAGS=-lncurses -lcurses

saucer: saucer.c saucer.h
	$(CC) -o saucer saucer.c $(CFLAGS) $(LDFLAGS)
clean:
	rm saucer
