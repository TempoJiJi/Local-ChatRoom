CC ?= gcc
CFLAGS ?= -lrt -D_XOPEN_SOURCE=500 -lpthread -g

EXEC = server client
all: $(EXEC)

server: mailbox_toolkit.o
	$(CC) $@.c mailbox_toolkit.o -o $@ $(CFLAGS)

client: mailbox_toolkit.o
	$(CC) $@.c mailbox_toolkit.o -o $@ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(EXEC) *.o *.s

