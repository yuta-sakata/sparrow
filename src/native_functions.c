#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "native_functions.h"

// 创建原生函数对象
NativeFunction *createNativeFn(const char *name, int arity, Value (*function)(int, Value *))
{
    NativeFunction *native = (NativeFunction *)malloc(sizeof(NativeFunction));
    if (native == NULL)
    {
        fprintf(stderr, "ERROR: Failed to allocate memory for native function\n");
        return NULL;
    }

    // 使用安全的字符串复制
    if (name != NULL)
    {
        size_t len = strlen(name) + 1;
        native->name = malloc(len);
        if (native->name == NULL)
        {
            fprintf(stderr, "ERROR: Failed to allocate memory for native function name\n");
            free(native);
            return NULL;
        }
        memcpy(native->name, name, len);
    }
    else
    {
        native->name = NULL;
    }

    native->arity = arity;
    native->function = function;
    return native;
}
// 注册单个原生函数
static void registerNativeFunction(Interpreter *interpreter, const char *name, int arity, Value (*function)(int, Value *))
{
    // 验证参数
    if (interpreter == NULL || interpreter->globals == NULL || name == NULL || function == NULL)
    {
        fprintf(stderr, "ERROR: Invalid arguments to registerNativeFunction\n");
        return;
    }

    // 创建原生函数对象
    NativeFunction *native = createNativeFn(name, arity, function);
    if (native == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create native function\n");
        return;
    }

    // 创建值并定义变量
    Value nativeValue = createNativeFunction(native);
    defineVariable(interpreter->globals, name, nativeValue);
}

// 注册所有原生函数
void registerAllNativeFunctions(Interpreter *interpreter)
{
    // 安全检查
    if (interpreter == NULL || interpreter->globals == NULL)
    {
        printf("ERROR: Cannot register native functions, interpreter or globals is NULL\n");
        return;
    }

    // 基础函数
    registerNativeFunction(interpreter, "print", -1, printNative);
    registerNativeFunction(interpreter, "println", -1, printlnNative);
    registerNativeFunction(interpreter, "clock", 0, clockNative);
    registerNativeFunction(interpreter, "type", 1, typeNative);
    registerNativeFunction(interpreter, "input", -1, inputNative);

    // 数组相关函数
    registerNativeFunction(interpreter, "length", 1, lengthNative);
    registerNativeFunction(interpreter, "push", 2, pushNative);
    registerNativeFunction(interpreter, "pop", 1, popNative);
    registerNativeFunction(interpreter, "popArray", 1, popArrayNative);
    registerNativeFunction(interpreter, "slice", -1, sliceNative); // -1表示可变参数（2或3个）

    // 标记解释器支持 main 函数
    interpreter->hasMainFunction = false;
    interpreter->mainFunction = NULL;
}

// 实现 print 原生函数
Value printNative(int argCount, Value *args)
{
    // 添加安全检查
    if (args == NULL && argCount > 0)
    {
        printf("ERROR: NULL args pointer passed to printNative\n");
        return createNull();
    }

    for (int i = 0; i < argCount; i++)
    {
        printValue(args[i]);
        if (i < argCount - 1)
            printf(" ");
    }
    return createNull();
}

Value printlnNative(int argCount, Value *args)
{
    // 添加安全检查
    if (args == NULL && argCount > 0)
    {
        printf("ERROR: NULL args pointer passed to printlnNative\n");
        return createNull();
    }

    for (int i = 0; i < argCount; i++)
    {
        printValue(args[i]);
        if (i < argCount - 1)
            printf(" ");
    }
    printf("\n");
    return createNull();
}

// input 原生函数
Value inputNative(int argCount, Value *args)
{
    // 检查参数数量（0个或1个）
    if (argCount > 1)
    {
        printf("ERROR: input() takes at most 1 argument\n");
        return createNull();
    }

    // 如果有参数，打印提示信息
    if (argCount == 1)
    {
        if (args[0].type == VAL_STRING)
        {
            printf("%s", args[0].as.string);
        }
        else
        {
            // 如果参数不是字符串，先转换为字符串显示
            printValue(args[0]);
        }
        fflush(stdout); // 确保提示信息立即显示
    }

    // 分配缓冲区来存储用户输入
    char *buffer = malloc(1024);
    if (buffer == NULL)
    {
        printf("ERROR: Memory allocation failed for input buffer\n");
        return createNull();
    }

    // 读取用户输入
    if (fgets(buffer, 1024, stdin) == NULL)
    {
        free(buffer);
        return createString(""); // 如果读取失败，返回空字符串
    }

    // 移除换行符
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    // 创建字符串值
    Value result = createString(buffer);
    free(buffer);

    return result;
}

// 实现 clock 原生函数
Value clockNative(int argCount, Value *args)
{
    if (argCount != 0)
    {
        printf("ERROR: clock() takes no arguments\n");
        return createNull();
    }
    
    return createNumber((double)time(NULL));
}

// 实现 type 原生函数
Value typeNative(int argCount, Value *args)
{
    if (argCount != 1)
    {
        return createString("ERROR: type() expects exactly 1 argument");
    }

    // 添加参数验证
    if (args == NULL)
    {
        return createString("ERROR: NULL arguments passed to type()");
    }

    Value arg = args[0];
    const char *typeName;

    switch (arg.type)
    {
    case VAL_NUMBER:
        // 检查是否为整数
        if (arg.as.number == (int)arg.as.number)
        {
            typeName = "int";
        }
        else
        {
            typeName = "float";
        }
        break;
    case VAL_STRING:
        typeName = "string";
        break;
    case VAL_BOOL:
        typeName = "bool";
        break;
    case VAL_NULL:
        typeName = "null";
        break;
    case VAL_ARRAY:
        typeName = "array";
        break;
    case VAL_FUNCTION:
        typeName = "function";
        break;
    case VAL_NATIVE_FUNCTION:
        typeName = "native_function";
        break;
    case VAL_ENUM_VALUE:
        typeName = "enum";
        break;
    default:
        typeName = "unknown";
        break;
    }

    return createString(typeName);
}

// 实现数组长度原生函数
Value lengthNative(int argCount, Value *args)
{
    // 参数验证
    if (argCount != 1)
    {
        return createString("Error: length() requires exactly one argument");
    }

    if (args == NULL)
    {
        return createString("Error: NULL arguments passed to length()");
    }

    // 检查参数类型
    if (args[0].type == VAL_ARRAY)
    {
        if (args[0].as.array == NULL)
        {
            return createNumber(0);
        }
        return createNumber((double)args[0].as.array->count);
    }
    else if (args[0].type == VAL_STRING)
    {
        if (args[0].as.string == NULL)
        {
            return createNumber(0);
        }
        return createNumber((double)strlen(args[0].as.string));
    }
    else
    {
        return createString("Error: length() can only be called on arrays or strings");
    }
}

// 实现数组push原生函数
Value pushNative(int argCount, Value *args)
{
    if (argCount != 2)
    {
        printf("ERROR: push() requires exactly two arguments (array, element)\n");
        return createNull();
    }

    if (args == NULL)
    {
        printf("ERROR: NULL arguments passed to push()\n");
        return createNull();
    }

    // 检查第一个参数是否为数组
    if (args[0].type != VAL_ARRAY)
    {
        printf("ERROR: first argument to push() must be an array\n");
        return createNull();
    }

    if (args[0].as.array == NULL)
    {
        printf("ERROR: NULL array passed to push()\n");
        return createNull();
    }

    // 创建数组的副本以避免修改原数组
    Value arrayValue = copyValue(args[0]);
    if (arrayValue.type == VAL_NULL || arrayValue.as.array == NULL)
    {
        printf("ERROR: failed to copy array\n");
        return createNull();
    }

    Array *array = arrayValue.as.array;

    // 确保数组有足够容量
    if (array->count >= array->capacity)
    {
        int newCapacity = array->capacity * 2;
        if (newCapacity < 8)
            newCapacity = 8;

        Value *newElements = (Value *)realloc(array->elements, sizeof(Value) * newCapacity);
        if (newElements == NULL)
        {
            printf("ERROR: failed to expand array\n");
            freeValue(arrayValue);
            return createNull();
        }
        array->elements = newElements;

        // 初始化新分配的内存
        for (int i = array->capacity; i < newCapacity; i++)
        {
            array->elements[i] = createNull();
        }

        array->capacity = newCapacity;
    }

    // 创建元素的副本并添加到数组
    Value elementCopy = copyValue(args[1]);
    array->elements[array->count] = elementCopy;
    array->count++;

    // 返回修改后的数组副本
    return arrayValue;
}

// 实现数组pop原生函数 - 返回弹出的元素，但不修改原数组
Value popNative(int argCount, Value *args)
{
    if (argCount != 1)
    {
        return createString("Error: pop() requires exactly one argument");
    }

    if (args == NULL)
    {
        return createString("Error: NULL arguments passed to pop()");
    }

    if (args[0].type != VAL_ARRAY)
    {
        return createString("Error: pop() can only be called on arrays");
    }

    if (args[0].as.array == NULL)
    {
        return createString("Error: NULL array passed to pop()");
    }

    Array *array = args[0].as.array;

    if (array->count == 0)
    {
        return createNull();
    }

    // 返回最后一个元素的副本
    return copyValue(array->elements[array->count - 1]);
}

// 实现数组popArray原生函数 - 返回移除最后元素的新数组
Value popArrayNative(int argCount, Value *args)
{
    if (argCount != 1)
    {
        return createString("Error: popArray() requires exactly one argument");
    }

    if (args == NULL)
    {
        return createString("Error: NULL arguments passed to popArray()");
    }

    if (args[0].type != VAL_ARRAY)
    {
        return createString("Error: popArray() can only be called on arrays");
    }

    if (args[0].as.array == NULL)
    {
        return createString("Error: NULL array passed to popArray()");
    }

    Array *originalArray = args[0].as.array;

    if (originalArray->count == 0)
    {
        // 返回空数组的副本
        return copyValue(args[0]);
    }

    // 创建数组的副本
    Value arrayValue = copyValue(args[0]);
    if (arrayValue.type == VAL_NULL || arrayValue.as.array == NULL)
    {
        printf("ERROR: failed to copy array\n");
        return createNull();
    }

    Array *array = arrayValue.as.array;

    // 释放最后一个元素
    freeValue(array->elements[array->count - 1]);
    array->count--;

    // 返回修改后的数组副本
    return arrayValue;
}

// 实现数组切片原生函数
Value sliceNative(int argCount, Value *args)
{
    // 参数验证：slice(array, start) 或 slice(array, start, end)
    if (argCount < 2 || argCount > 3)
    {
        return createString("Error: slice() requires 2 or 3 arguments (array, start, [end])");
    }

    if (args == NULL)
    {
        return createString("Error: NULL arguments passed to slice()");
    }

    // 检查第一个参数是否为数组
    if (args[0].type != VAL_ARRAY)
    {
        return createString("Error: first argument to slice() must be an array");
    }

    if (args[0].as.array == NULL)
    {
        return createString("Error: NULL array passed to slice()");
    }

    // 检查start参数是否为数字
    if (args[1].type != VAL_NUMBER)
    {
        return createString("Error: start index must be a number");
    }

    Array *sourceArray = args[0].as.array;
    int start = (int)args[1].as.number;
    int end = sourceArray->count; // 默认到数组末尾

    // 如果提供了end参数
    if (argCount == 3)
    {
        if (args[2].type != VAL_NUMBER)
        {
            return createString("Error: end index must be a number");
        }
        end = (int)args[2].as.number;
    }

    // 处理负数索引
    if (start < 0)
    {
        start = sourceArray->count + start;
    }
    if (end < 0)
    {
        end = sourceArray->count + end;
    }

    // 边界检查
    if (start < 0)
        start = 0;
    if (end > sourceArray->count)
        end = sourceArray->count;
    if (start >= end)
    {
        // 返回空数组
        return createArray(sourceArray->elementType, 0);
    }

    // 创建新数组
    int sliceLength = end - start;
    Value newArray = createArray(sourceArray->elementType, sliceLength);
    if (newArray.type == VAL_NULL)
    {
        return createString("Error: failed to create slice array");
    }

    // 复制元素到新数组
    for (int i = start; i < end; i++)
    {
        arrayPush(newArray.as.array, sourceArray->elements[i]);
    }

    return newArray;
}