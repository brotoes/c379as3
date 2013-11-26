CC=gcc
CFLAGS=-pthread
LDFLAGS=-lncurses

saucer: saucer.c saucer.h
	$(CC) -o saucer saucer.c $(CFLAGS) $(LDFLAGS)

clean:
	rm saucer
