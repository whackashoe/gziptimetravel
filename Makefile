CFLAGS= -g -pedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
CC= gcc
BINDIR= /usr/local/bin/

all: gziptimetravel

.o:
	$(CC) $(CFLAGS) -o $@ $<

gziptimetravel.o: gziptimetravel.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: all
	install -m 755 gziptimetravel $(BINDIR)

clean:
	rm gziptimetravel *.o
