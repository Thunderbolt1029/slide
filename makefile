.POSIX:
CC = gcc
EXT = c
EXEC = slide

CFLAGS = -W -O 
LDLIBS = -lncurses

SRC_DIR = src
OBJ_DIR = obj

SRC_FILES := $(wildcard $(SRC_DIR)/*.$(EXT))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.$(EXT),$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(EXEC)

$(EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(EXT) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)/ $(EXEC)
