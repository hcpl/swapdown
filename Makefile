CFLAGS=-g -Wall -Wextra -std=c89


all: swapdown

install:
	install -p -m 0755 swapdown /usr/local/bin/

install-suid:
	install -p -m 4755 swapdown /usr/local/bin/
