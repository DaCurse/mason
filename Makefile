CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra
LDFLAGS = -lcjson

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
HEADERS = mason.h mason_arena.h
SOURCES = mason.c mason_arena.c
LIB_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))
EXAMPLES = $(filter-out examples/utils.c,$(wildcard examples/*.c))
BINS = $(patsubst examples/%.c,$(BUILD_DIR)/mason_%,$(EXAMPLES))

all: $(BINS)

$(BUILD_DIR)/mason_%: $(OBJ_DIR)/%.o $(OBJ_DIR)/utils.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: examples/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

run-%: $(BUILD_DIR)/mason_%
	./$<

run: all
	@for f in $(BINS); do \
		printf "\n==> %s\n" "$$f"; \
		"$$f"; \
	done

clean:
	rm -rf $(BUILD_DIR)

format:
	clang-format -i examples/*.c *.c *.h

san: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
san: LDFLAGS += -fsanitize=address,undefined
san: clean all

.PHONY: all run run-% clean format san
