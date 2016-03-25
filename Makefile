.PHONY: all clean test

SOURCES=$(wildcard src/*.cc)
OBJECTS=$(addsuffix .o, $(SOURCES))

LUA_SCRIPTS=$(shell find scripts -type f -name '*.lua')
LUA_SCRIPTS_C=$(patsubst scripts/%.lua, src/embedded/%.c, $(LUA_SCRIPTS))

TARGET=bblua

INCLUDE_PATHS=-Isrc/lua/src
LIB_PATHS=-Lsrc/lua/src

CXXFLAGS=-g -Wall -Werror -D__STDC_LIMIT_MACROS

all: $(TARGET) luaminify

%.cc.o: %.cc
	${CXX} $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

# Generate strings from Lua files.
src/embedded/%.c: scripts/%.lua
	@mkdir -p "$(@D)"
	xxd -i $^ $@

src/embedded.cc.o: $(LUA_SCRIPTS_C)

$(TARGET): $(OBJECTS) src/lua/src/liblua.a
	${CXX} $(OBJECTS) $(LIB_PATHS) -llua -o $@

src/lua/src/liblua.a:
	${MAKE} -C src/lua posix

luaminify: tools/luaminify.cc.o
	${CXX} $^ -o $@

test: $(TARGET)
	@./test

clean:
	$(RM) $(TARGET) luaminify $(OBJECTS) $(LUA_SCRIPTS_C)
	${MAKE} -C src/lua clean
