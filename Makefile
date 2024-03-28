CC=gcc
CFLAGS= -g -std=c11 -pedantic -Wall -Wextra
.PHONY: clean run

client:
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -f client
