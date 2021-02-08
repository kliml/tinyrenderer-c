CC=gcc
TARGET = renderer
LDFLAGS = -lm
SRC := src
OBJ := build
SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

LDFLAGS = -lm

all: $(SOURCES) $(TARGET)

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p build
	$(CC) -I$(SRC) -c $< -o $@ $(LDFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJECTS) myprog build