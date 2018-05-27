#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

clean:
	$(RM) $(RMFILES) *.o
