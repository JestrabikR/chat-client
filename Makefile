CC=gcc
CFLAGS= -g -std=c11 -pedantic -Wall -Wextra
.PHONY: clean run

client: helpers.c commands.c response.c sent_messages_queue.c
	$(CC) $(CFLAGS) $@.c $^ -o $@

clean:
	rm -f client
