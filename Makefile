CC=clang
CFLAGS = -Wall -Wpedantic -Wextra -std=c99 -ggdb

SRC = src
OBJ = objs

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR = bin
BIN = $(BINDIR)/ll

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CINCLUDES) $^ -o $@ $(CLIBS)

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CINCLUDES) -c $< -o $@

clean:
	rm -rf $(BINDIR) $(OBJ)
	rm -rf transpiler_out

$(OBJ):
	@mkdir -p $@

run: $(BIN)
	@mkdir -p transpiler_out
	@cp -pr src/bundle/* transpiler_out
	$(BIN)

transpiler_out/transpiled_file.c: run

test: transpiler_out/transpiled_file.c
	@$(MAKE) -C transpiler_out

