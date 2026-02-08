 ###########################################################################
 #      Copyright (c) 2026 - Yassin Achengli <achengli@github.com>
 #
 #      This software is under BSD license.
 ###########################################################################
 #      + File: Makefile
 ###########################################################################
LUA ?= lua5.1
LUA_CFLAGS = 
LUA_LIBS = -llua -lm -ldl
OUT = leregex

MAJOR ?= 1
MINOR ?= 0
RELEASE ?= 0

CFLAGS ?= -fPIC

# Compatibility with github workflows (ubuntu)
OS_ID := $(shell . /etc/os-release; echo $$ID)
ifeq ($(OS_ID),ubuntu)
	LUA_CFLAGS += -I/usr/include/lua5.4
endif

all: extract_version $(OUT).so 

%.o: %.c
	$(CC) -c $(CFLAGS) $(LUA_CFLAGS) -o $@ $<

$(OUT).so: $(OUT).o
	$(CC) -shared $(OUT).o -o $@ $(LUA_LIBS)

clean:
	rm -rf $(OUT).so *.o *.rock

extract_version:
	sh ./../scripts/extract_version_from_rockspec.sh --export

.PHONY: extract_version
