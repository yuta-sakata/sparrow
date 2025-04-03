#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "environment.h" // 确保包含这个

// 创建空值
Value createNull()
{
    Value value;
    value.type = VAL_NULL;
    return value;
}

// 创建布尔值
Value createBool(bool value)
{
    Value val;
    val.type = VAL_BOOL;
    val.as.boolean = value;
    return val;
}

// 创建数值
Value createNumber(double value)
{
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = value;
    return val;
}

// 创建字符串
Value createString(const char *value)
{
    Value val;
    val.type = VAL_STRING;
    
    // 处理NULL输入
    if (value == NULL) {
        printf("WARNING: NULL string passed to createString\n");
        val.as.string = malloc(1);  // 手动分配内存而不是用strdup
        if (val.as.string == NULL) {
            printf("ERROR: Failed to allocate memory for empty string\n");
            val.type = VAL_NULL;
            return val;
        }
        val.as.string[0] = '\0';  // 设置为空字符串
        return val;
    }
    
    // 计算字符串长度并检查有效性
    size_t len = strlen(value);
    
    // 手动分配内存并复制，而不是使用strdup
    val.as.string = (char*)malloc(len + 1);  // +1 用于 null 终止符
    if (val.as.string == NULL) {
        val.type = VAL_NULL;
        return val;
    }
    
    // 手动复制字符串
    memcpy(val.as.string, value, len);
    val.as.string[len] = '\0';  // 确保正确终止字符串
    
    return val;
}

// 创建函数值
Value createFunction(Function *function)
{
    Value val;
    val.type = VAL_FUNCTION;
    val.as.function = function;
    return val;
}

// 创建原生函数值
Value createNativeFunction(NativeFunction *function)
{
    Value val;
    val.type = VAL_NATIVE_FUNCTION;
    val.as.nativeFunction = function;
    return val;
}

// 值比较
bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type)
        return false;

    switch (a.type)
    {
    case VAL_NULL:
        return true;
    case VAL_BOOL:
        return a.as.boolean == b.as.boolean;
    case VAL_NUMBER:
        return a.as.number == b.as.number;
    case VAL_STRING:
        return strcmp(a.as.string, b.as.string) == 0;
    case VAL_FUNCTION:
        return a.as.function == b.as.function;
    case VAL_NATIVE_FUNCTION:
        return a.as.nativeFunction == b.as.nativeFunction;
    }

    return false;
}

// 打印值
void printValue(Value value)
{
    switch (value.type)
    {
    case VAL_NULL:
        printf("null");
        break;
    case VAL_BOOL:
        printf(value.as.boolean ? "true" : "false");
        break;
    case VAL_NUMBER:
        printf("%g", value.as.number);
        break;
    case VAL_STRING:
        if (value.as.string != NULL)
        {
            printf("\"%s\"", value.as.string);
        }
        else
        {
            printf("\"<null>\"");
        }
        break;
    case VAL_FUNCTION:
        printf("<function %s>", value.as.function->name);
        break;
    case VAL_NATIVE_FUNCTION:
        printf("<native function %s>", value.as.nativeFunction->name);
        break;
    }
}

// 复制值
Value copyValue(Value value)
{
    switch (value.type)
    {
    case VAL_STRING:
        return createString(value.as.string);
    case VAL_FUNCTION:
    {
        // 复制函数需要更复杂的处理，这里只是简单引用
        return value;
    }
    case VAL_NATIVE_FUNCTION:
        // 原生函数通常不需要复制
        return value;
    default:
        // 对于简单值类型，直接复制
        return value;
    }
}

// 释放值
void freeValue(Value value)
{
    switch (value.type)
    {
    case VAL_STRING:
        if (value.as.string != NULL)
        {
            free(value.as.string);
        }
        break;

    case VAL_FUNCTION:
        // 释放函数资源
        if (value.as.function != NULL)
        {
            if (value.as.function->name != NULL)
            {
                free(value.as.function->name);
            }

            // 释放参数名
            if (value.as.function->paramNames != NULL)
            {
                for (int i = 0; i < value.as.function->arity; i++)
                {
                    if (value.as.function->paramNames[i] != NULL)
                    {
                        free(value.as.function->paramNames[i]);
                    }
                }
                free(value.as.function->paramNames);
            }

            // 注意：不释放 body 和 closure，它们由其他部分管理
            free(value.as.function);
        }
        break;

    case VAL_NATIVE_FUNCTION:
        // 原生函数需要释放结构体，但不释放函数指针
        if (value.as.nativeFunction != NULL)
        {

            // 安全释放原生函数名称
            if (value.as.nativeFunction->name != NULL)
            {
                free(value.as.nativeFunction->name);
                value.as.nativeFunction->name = NULL; // 防止悬空指针
            }

            // 释放原生函数结构体本身
            free(value.as.nativeFunction);
        }
        break;

    default:
        // 其他类型不需要释放
        break;
    }
}
// 检查值是否与类型注解兼容
bool isValueCompatibleWithType(Value value, TypeAnnotation type)
{
    if (type == TYPE_ANY)
        return true;

    switch (type)
    {
    case TYPE_VOID:
        return value.type == VAL_NULL;

    case TYPE_INT:
    case TYPE_FLOAT:
        return value.type == VAL_NUMBER;

    case TYPE_STRING:
        return value.type == VAL_STRING;

    case TYPE_BOOL:
        return value.type == VAL_BOOL;

    case TYPE_FUNCTION:
        return value.type == VAL_FUNCTION || value.type == VAL_NATIVE_FUNCTION;

    default:
        return false;
    }
}