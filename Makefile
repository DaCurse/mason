CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra
LDFLAGS = -lcjson

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
HEADERS = mason.h mason_multi.h backends/cjson.h
EXAMPLES = $(filter-out examples/utils.c,$(wildcard examples/*.c))
BINS = $(patsubst examples/%.c,$(BUILD_DIR)/mason_%,$(EXAMPLES))
UTILS_OBJ = $(OBJ_DIR)/utils.o

all: $(BINS)

$(BUILD_DIR)/mason_%: $(OBJ_DIR)/%.o $(UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

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
	clang-format -i examples/*.c *.h backends/*.h

san: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
san: LDFLAGS += -fsanitize=address,undefined
san: clean all

.PHONY: all run run-% clean format san
