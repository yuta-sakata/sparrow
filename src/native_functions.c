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
    registerNativeFunction(interpreter, "clock", 0, clockNative);
    registerNativeFunction(interpreter, "type", 1, typeNative);

    // 标记解释器支持 main 函数
    interpreter->hasMainFunction = false;
    interpreter->mainFunction = NULL;

    // // I/O 函数
    // registerNativeFunction(interpreter, "readLine", 0, readLineNative);
    // registerNativeFunction(interpreter, "readFile", 1, readFileNative);
    // registerNativeFunction(interpreter, "writeFile", 2, writeFileNative);

    // // 数学函数
    // registerNativeFunction(interpreter, "sqrt", 1, sqrtNative);
    // registerNativeFunction(interpreter, "sin", 1, sinNative);
    // registerNativeFunction(interpreter, "cos", 1, cosNative);
    // registerNativeFunction(interpreter, "random", 0, randomNative);

    // // 字符串函数
    // registerNativeFunction(interpreter, "strlen", 1, strLenNative);
    // registerNativeFunction(interpreter, "substring", 3, subStringNative);
    // registerNativeFunction(interpreter, "concat", -1, strConcatNative);
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
    printf("\n");
    return createNull();
}

// 实现 clock 原生函数
Value clockNative(int argCount, Value *args)
{
    return createNumber((double)time(NULL));
}

// 实现 type 原生函数
Value typeNative(int argCount, Value *args)
{
    if (argCount != 1 || args == NULL)
    {
        return createString("Error: type() requires exactly one argument");
    }

    char *typeStr;
    switch (args[0].type)
    {
    case VAL_NULL:
        typeStr = "null";
        break;
    case VAL_BOOL:
        typeStr = "boolean";
        break;
    case VAL_NUMBER:
        typeStr = "number";
        break;
    case VAL_STRING:
        typeStr = "string";
        break;
    case VAL_FUNCTION:
        typeStr = "function";
        break;
    case VAL_NATIVE_FUNCTION:
        typeStr = "native function";
        break;
    default:
        typeStr = "unknown";
        break;
    }

    return createString(typeStr);
}

// ... 其他原生函数实现 ...