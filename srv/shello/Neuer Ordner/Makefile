#!/usr/bin/make
.SUFFIXES:
SRC = prog.c
TAR = prog
PCK = lab-2.zip

CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: $(TAR)

run: $(TAR)
	./prog

pack:
	zip $(PCK) $(SRC) Makefile

clean:
	$(RM) $(RMFILES) $(TAR) $(PCK)
