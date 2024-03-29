CC=gcc
CFLAGS= -g -std=c11 -pedantic -Wall -Wextra
.PHONY: clean run

client: messages.c udp/udp.c helpers.c commands.c
	$(CC) $(CFLAGS) $@.c $^ -o $@

clean:
	rm -f client
