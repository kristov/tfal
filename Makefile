CC := gcc
CFLAGS := -Wall -Werror -ggdb

all: memlen

memlen: memlen.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f memlen
