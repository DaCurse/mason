CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra
LDFLAGS = -lcjson

SRCS = examples/discord.c
OBJS = $(SRCS:.c=.o)
HEADERS = mason.h mason_multi.h backends/cjson.h
TARGET = mason_demo

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

format:
	clang-format -i examples/*.c *.h backends/*.h

san: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
san: LDFLAGS += -fsanitize=address,undefined
san: clean $(TARGET)
	./$(TARGET)

.PHONY: all run clean format san
