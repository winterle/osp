#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean

TAR = ult
PCK = lab-4.zip
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror -Wno-unused-value -Wno-deprecated



BUILD_DIR = VM
COPY_FILES = $(BUILD_DIR)/array.c $(BUILD_DIR)/array.h $(BUILD_DIR)/test.c $(BUILD_DIR)/ult.c $(BUILD_DIR)/ult.h $(BUILD_DIR)/owntest.c

$(BUILD_DIR)/array.c: array.c
$(BUILD_DIR)/array.h: array.h
$(BUILD_DIR)/ult.c: ult.c
$(BUILD_DIR)/ult.h: ult.h
$(BUILD_DIR)/test.c: test.c
$(BUILD_DIR)/owntest.c: owntest.c

$(BUILD_DIR):
	mkdir -p $@


$(BUILD_DIR)/%:
	cp -f $< $@

	
copy: $(BUILD_DIR) $(COPY_FILES)
	

%.o: %.c %.h
	cd VM && $(CC) $(CFLAGS) $< -o $@

%.o: %.c
	cd VM && $(CC) $(CFLAGS) $< -o $@

ult: array.o test.o ult.o
	cd VM && $(CC) $^ -o $@

all: copy $(TAR)

allo: copy ulto

ulto: array.o owntest.o ult.o
	cd VM && $(CC) $^ -o $@

run: all
	cd VM && clear && clear && ./ult

runo: allo
	cd VM && clear && clear && ./ulto
	
val: all
	cd VM && clear && G_SLICE=always-malloc G_DEBUG=gc-friendly  valgrind -v --tool=memcheck --leak-check=full --show-leak-kinds=all --num-callers=40 --log-file=valgrind.log ./ult

valo: allo
	cd VM && clear && G_SLICE=always-malloc G_DEBUG=gc-friendly  valgrind -v --tool=memcheck --leak-check=full --show-leak-kinds=all --num-callers=40 --log-file=valgrind.log ./ulto
	
	

pack: clean
	zip $(PCK) *.c *.h Makefile -x "./.*"

clean:
	cd VM && $(RM) $(RMFILES) $(OBJ) $(TAR)
