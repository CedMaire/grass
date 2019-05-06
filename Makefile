SRCDIR   = src
BINDIR   = bin
INCLUDES = include

CC=gcc
CFLAGS=-Wall -Wextra -g -fno-stack-protector -z execstack -pthread -std=gnu11 -m32 -I $(INCLUDES)/
DEPS = $(wildcard $(INCLUDES)/%.h)

all: mkdir_conf $(BINDIR)/client $(BINDIR)/server $(DEPS)

mkdir_conf:
	mkdir bin 2>/dev/null || true
	cp grass.conf bin/grass.conf

$(BINDIR)/client: $(SRCDIR)/client.c
	$(CC) $(CFLAGS) $(SRCDIR)/grass.c $< -o $@

$(BINDIR)/server: $(SRCDIR)/server.c
	$(CC) $(CFLAGS) $(SRCDIR)/grass.c $< -o $@

.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server
