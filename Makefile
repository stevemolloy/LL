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
	rm transpiled_file.c
	rm a.out

$(OBJ):
	@mkdir -p $@

run: $(BIN)
	$(BIN)

test: $(BIN)
	$(BIN)
	$(CC) transpiled_file.c -ggdb -Wall -Wextra -std=c99 -Wpedantic && ./a.out

