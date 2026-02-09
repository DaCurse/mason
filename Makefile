CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra
LDFLAGS = -lcjson

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
UTILS_SRC = examples/utils.c
UTILS_OBJ = $(patsubst examples/%.c,$(OBJ_DIR)/%.o,$(UTILS_SRC))
EXAMPLES = $(filter-out $(UTILS_SRC),$(wildcard examples/*.c))
EXAMPLE_OBJS = $(patsubst examples/%.c,$(OBJ_DIR)/%.o,$(EXAMPLES))
EXAMPLE_BINS = $(patsubst examples/%.c,$(BUILD_DIR)/mason_%,$(EXAMPLES))
HEADERS = mason.h mason_multi.h backends/cjson.h

all: $(EXAMPLE_BINS)

$(EXAMPLE_BINS): $(BUILD_DIR)/mason_%: $(OBJ_DIR)/%.o $(UTILS_OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $< $(UTILS_OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: examples/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

run-%: $(BUILD_DIR)/mason_%
	./$(BUILD_DIR)/mason_$*

run: all
	@for f in $(EXAMPLES); do \
		bin=$(BUILD_DIR)/mason_$$(basename $$f .c); \
		printf "\n==> %s\n" "$$bin"; \
		"$$bin"; \
	done

clean:
	rm -rf $(BUILD_DIR)

format:
	clang-format -i examples/*.c *.h backends/*.h

san: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
san: LDFLAGS += -fsanitize=address,undefined
san: clean all

.PHONY: all run run-% clean format san
