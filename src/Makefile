LUA ?= lua5.1
LUA_CFLAGS = 
LUA_LIBS = -llua -lm -ldl
OUT = leregex

CFLAGS ?= -fPIC

all: $(OUT).so

%.o: %.c
	$(CC) -c $(CFLAGS) $(LUA_CFLAGS) -o $@ $<

$(OUT).so: $(OUT).o
	$(CC) -shared $(OUT).o -o $@ $(LUA_LIBS)

clean:
	rm -rf $(OUT).so *.o *.rock
