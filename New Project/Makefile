CC      = gcc
CFLAGS  = -Wall -Wextra $(shell pkg-config --cflags gtk+-3.0)
LIBS    = $(shell pkg-config --libs gtk+-3.0)

TARGET  = backup_utility.exe

SRC_DIRS = src/main src/core src/rendering src/utils
SRCS     = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS     = $(patsubst %.c,build/%.o,$(SRCS))

INCLUDE  = -Iinclude

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
