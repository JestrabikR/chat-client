CC=gcc
CFLAGS= -g -std=c11 -pedantic -Wall -Wextra
.PHONY: clean run

ipk24chat-client: helpers.c commands.c response.c sent_messages_queue.c tcp.c
	$(CC) $(CFLAGS) main.c $^ -o $@

clean:
	rm -f ipk24chat-client
