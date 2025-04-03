CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g3 -gdwarf-4 -fno-omit-frame-pointer
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
OUTPUT_DIR = output
TARGET = $(OUTPUT_DIR)/sparrow
LDFLAGS = -lm  # 添加数学库链接

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: directories $(TARGET)

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OUTPUT_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -g -o $@ $^ $(LDFLAGS)  # 添加 LDFLAGS

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)