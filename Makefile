PCKGS = gl freetype2 glew libmagic harfbuzz
CFLAGS=-Wall -std=c23 -pedantic -g -fcolor-diagnostics# -fsanitize=address
LDFLAGS = -lm -lGL -lglfw `pkg-config --libs $(PCKGS)`
INCS=-I include/ `pkg-config --cflags $(PCKGS)`

COMPILER = clang

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = .

# Files
SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))


TARGET = app

all: $(TARGET) $(BUILD_DIR)

run: all
	@./$(TARGET)

debug: all
	gdb ./$(TARGET)

$(TARGET): $(OBJ_FILES)
	$(COMPILER) $(CFLAGS) $(INCS)  -o $@ $^ $(LDFLAGS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) $(INCS) -c -o "$@" "$<"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
