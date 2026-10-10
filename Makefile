CC         ?= cc
PKG_CONFIG ?= pkg-config
VALGRIND   ?= valgrind

VALGRIND_FLAGS ?= --leak-check=full --show-leak-kinds=all --track-origins=yes

BIN := rt_music_viz
SRC := $(wildcard src/*.c)

# gnu17 rather than strict c17: the PipeWire/SPA headers use GNU extensions.
CFLAGS += -std=gnu17 -Wall -Wextra -g -Iinclude
CFLAGS += $(shell $(PKG_CONFIG) --cflags libpipewire-0.3)
LDLIBS += $(shell $(PKG_CONFIG) --libs libpipewire-0.3)

# raylib ships no pkg-config or CMake package here, so link it directly.
CFLAGS += -I/usr/local/include
LDLIBS += -L/usr/local/lib -lraylib -lGL -lX11 -ldl -lpthread -lm -lrt

.PHONY: all clean run compile_commands valgrind

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDLIBS) -o $@

run: $(BIN)
	./$(BIN)

# Run the binary under valgrind. The build already carries -g debug symbols.
valgrind: $(BIN)
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(BIN)

# Regenerate compile_commands.json for clangd. bear wraps a clean build so
# every compiler invocation is recorded.
compile_commands:
	bear --output compile_commands.json -- $(MAKE) clean all

clean:
	rm -f $(BIN) compile_commands.json
