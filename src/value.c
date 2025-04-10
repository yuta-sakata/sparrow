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
    if (value == NULL)
    {
        printf("WARNING: NULL string passed to createString\n");
        val.as.string = malloc(1);
        if (val.as.string == NULL)
        {
            printf("ERROR: Failed to allocate memory for empty string\n");
            val.type = VAL_NULL;
            return val;
        }
        val.as.string[0] = '\0'; // 设置为空字符串
        return val;
    }

    // 计算字符串长度并检查有效性
    size_t len = strlen(value);

    val.as.string = (char *)malloc(len + 1); // +1 用于 null 终止符
    if (val.as.string == NULL)
    {
        val.type = VAL_NULL;
        return val;
    }

    // 手动复制字符串
    memcpy(val.as.string, value, len);
    val.as.string[len] = '\0'; // 确保正确终止字符串

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
            printf("%s", value.as.string);
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
        if (value.as.string != NULL)
        {
            return createString(value.as.string); // 创建新的字符串副本
        }
        else
        {
            return createString(""); // 防止NULL指针
        }
    case VAL_FUNCTION:
    {
        // 深度复制函数对象
        Function *original = value.as.function;
        if (original == NULL)
        {
            return createNull();
        }

        // 分配新的函数结构
        Function *newFunction = (Function *)malloc(sizeof(Function));
        if (newFunction == NULL)
        {
            printf("ERROR: Failed to allocate memory for function copy\n");
            return createNull();
        }

        // 复制基本类型字段
        newFunction->arity = original->arity;
        newFunction->returnType = original->returnType;

        // 复制函数名
        if (original->name != NULL)
        {
            size_t nameLen = strlen(original->name);
            newFunction->name = (char *)malloc(nameLen + 1);
            if (newFunction->name == NULL)
            {
                free(newFunction);
                printf("ERROR: Failed to allocate memory for function name\n");
                return createNull();
            }
            strcpy(newFunction->name, original->name);
        }
        else
        {
            newFunction->name = NULL;
        }

        // 复制参数名
        newFunction->paramNames = NULL;
        if (original->arity > 0 && original->paramNames != NULL)
        {
            newFunction->paramNames = (char **)malloc(sizeof(char *) * original->arity);
            if (newFunction->paramNames == NULL)
            {
                free(newFunction->name);
                free(newFunction);
                printf("ERROR: Failed to allocate memory for param names\n");
                return createNull();
            }

            // 初始化为NULL，避免释放未分配内存的错误
            for (int i = 0; i < original->arity; i++)
            {
                newFunction->paramNames[i] = NULL;
            }

            // 复制每个参数名
            for (int i = 0; i < original->arity; i++)
            {
                if (original->paramNames[i] != NULL)
                {
                    size_t paramLen = strlen(original->paramNames[i]);
                    newFunction->paramNames[i] = (char *)malloc(paramLen + 1);
                    if (newFunction->paramNames[i] == NULL)
                    {
                        // 清理已分配资源
                        for (int j = 0; j < i; j++)
                        {
                            free(newFunction->paramNames[j]);
                        }
                        free(newFunction->paramNames);
                        free(newFunction->name);
                        free(newFunction);
                        printf("ERROR: Failed to allocate memory for param name\n");
                        return createNull();
                    }
                    strcpy(newFunction->paramNames[i], original->paramNames[i]);
                }
            }
        }

        // 复制参数类型（如果存在）
        newFunction->paramTypes = NULL;
        if (original->arity > 0 && original->paramTypes != NULL)
        {
            newFunction->paramTypes = (TypeAnnotation *)malloc(sizeof(TypeAnnotation) * original->arity);
            if (newFunction->paramTypes == NULL)
            {
                // 清理已分配资源
                for (int i = 0; i < original->arity; i++)
                {
                    if (newFunction->paramNames[i] != NULL)
                    {
                        free(newFunction->paramNames[i]);
                    }
                }
                free(newFunction->paramNames);
                free(newFunction->name);
                free(newFunction);
                printf("ERROR: Failed to allocate memory for param types\n");
                return createNull();
            }

            // 复制参数类型
            memcpy(newFunction->paramTypes, original->paramTypes, sizeof(TypeAnnotation) * original->arity);
        }

        // 对于body和closure保持引用，不进行深度复制
        newFunction->body = original->body;
        newFunction->closure = original->closure;

        // 创建新的函数值并返回
        Value newValue;
        newValue.type = VAL_FUNCTION;
        newValue.as.function = newFunction;
        return newValue;
    }
    case VAL_NATIVE_FUNCTION:
    {
        // 深度复制原生函数对象
        NativeFunction *original = value.as.nativeFunction;
        if (original == NULL)
        {
            return createNull();
        }

        // 分配新的原生函数结构
        NativeFunction *newNativeFunction = (NativeFunction *)malloc(sizeof(NativeFunction));
        if (newNativeFunction == NULL)
        {
            printf("ERROR: Failed to allocate memory for native function copy\n");
            return createNull();
        }

        // 复制函数指针和参数数量
        newNativeFunction->function = original->function;
        newNativeFunction->arity = original->arity;
        newNativeFunction->name = NULL;

        // 复制函数名
        if (original->name != NULL)
        {
            size_t nameLen = strlen(original->name);
            newNativeFunction->name = (char *)malloc(nameLen + 1);
            if (newNativeFunction->name == NULL)
            {
                free(newNativeFunction);
                printf("ERROR: Failed to allocate memory for native function name\n");
                return createNull();
            }
            strcpy(newNativeFunction->name, original->name);
        }

        // 创建新的原生函数值并返回
        Value newValue;
        newValue.type = VAL_NATIVE_FUNCTION;
        newValue.as.nativeFunction = newNativeFunction;
        return newValue;
    }
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