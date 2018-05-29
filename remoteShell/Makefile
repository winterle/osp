#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
.SILENT: run

TAR = cli/client srv/server
PCK = lab-3.zip

export CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror

all: $(TAR)

run: all
	gcc -std=gnu11 shell.c -o shell
	srv/server& echo $$! > .srv_pid
	echo "Client Ready:"
	cli/client
	kill `cat .srv_pid`
	$(RM) $(RMFILES) .srv_pid

pack: clean
	zip -r $(PCK) cli srv Makefile -x "*/.*"

clean:
	@$(MAKE) -C cli clean
	@$(MAKE) -C srv clean
	$(RM) $(RMFILES) $(PCK)

cli/client: cli/*.c cli/*.h
	@$(MAKE) -C $(@D)

srv/server: srv/*.c srv/*.h
	@$(MAKE) -C $(@D)


