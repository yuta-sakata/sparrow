CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
OUTPUT_DIR = output

# 核心源文件
CORE_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/lexer.c \
               $(SRC_DIR)/ast.c $(SRC_DIR)/environment.c $(SRC_DIR)/value.c \
               $(SRC_DIR)/native_functions.c $(SRC_DIR)/file_utils.c \
               $(SRC_DIR)/type_system.c

# 解析器模块源文件
PARSER_SOURCES = $(SRC_DIR)/parser/parser_core.c \
                 $(SRC_DIR)/parser/declaration_parser.c \
                 $(SRC_DIR)/parser/statement_parser.c \
                 $(SRC_DIR)/parser/expression_parser.c \
                 $(SRC_DIR)/parser/type_parser.c

# 解释器模块源文件
INTERPRETER_SOURCES = $(SRC_DIR)/interpreter/interpreter_core.c \
                      $(SRC_DIR)/interpreter/expression_evaluator.c \
                      $(SRC_DIR)/interpreter/binary_operations.c \
                      $(SRC_DIR)/interpreter/unary_operations.c \
                      $(SRC_DIR)/interpreter/array_operations.c \
                      $(SRC_DIR)/interpreter/function_calls.c \
                      $(SRC_DIR)/interpreter/statement_executor.c \
                      $(SRC_DIR)/interpreter/cast_operations.c


ALL_SOURCES = $(CORE_SOURCES) $(PARSER_SOURCES) $(INTERPRETER_SOURCES)

# 目标文件
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(ALL_SOURCES))

# 最终目标
TARGET = $(OUTPUT_DIR)/sparrow

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJECTS) | $(OUTPUT_DIR)
	$(CC) $(OBJECTS) -o $@ -lm

# 编译规则 - 处理嵌套目录
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 创建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/parser
	mkdir -p $(BUILD_DIR)/interpreter

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# 清理
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

# 运行测试
test: $(TARGET)
	./$(TARGET) test.spw

.PHONY: all clean test