CC=clang
CFLAGS = -Wall -Wpedantic -Wextra -std=c18 -ggdb
ifeq ($(CC), clang)
	CFLAGS +=  -fsanitize=undefined,address
endif

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

run: $(BIN)
	$(BIN)

