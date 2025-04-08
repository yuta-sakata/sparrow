#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "environment.h"
#include "interpreter.h"

// 运行时错误结构
typedef struct
{
    bool hadError;
    bool hadReturn;
    Value returnValue;
} RuntimeError;

// 静态变量用于跟踪运行时状态
static RuntimeError error;

// 运行时错误处理
static void runtimeError(Interpreter *interpreter, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(interpreter->errorMessage, sizeof(interpreter->errorMessage), format, args);
    va_end(args);

    interpreter->hadError = true;
    error.hadError = true;
}

// 初始化环境
void initEnvironment(Environment *env, Environment *enclosing)
{
    env->enclosing = enclosing;
    env->capacity = 8;
    env->count = 0;

    // 初始化名称和值数组
    env->names = (char **)malloc(sizeof(char *) * env->capacity);
    if (env->names == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    env->values = (Value *)malloc(sizeof(Value) * env->capacity);
    if (env->values == NULL)
    {
        free(env->names);
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // 初始化为NULL，避免后续比较出错
    for (int i = 0; i < env->capacity; i++)
    {
        env->names[i] = NULL;
    }
}

// 定义新变量
void defineVariable(Environment *env, const char *name, Value value)
{
    if (env == NULL || name == NULL)
    {
        fprintf(stderr, "ERROR: NULL parameter in defineVariable\n");
        return;
    }

    // 确保数组有足够空间
    if (env->count >= env->capacity)
    {
        int newCapacity = env->capacity * 2;
        char **newNames = (char **)realloc(env->names, sizeof(char *) * newCapacity);
        Value *newValues = (Value *)realloc(env->values, sizeof(Value) * newCapacity);

        if (newNames == NULL || newValues == NULL)
        {
            fprintf(stderr, "ERROR: Failed to expand environment arrays\n");
            if (newNames != NULL)
                free(newNames);
            if (newValues != NULL)
                free(newValues);
            return;
        }

        env->names = newNames;
        env->values = newValues;
        env->capacity = newCapacity;

        // 初始化新分配的空间
        for (int i = env->count; i < env->capacity; i++)
        {
            env->names[i] = NULL;
        }
    }

    // 先进行深度拷贝，然后检查结果
    size_t name_len = strlen(name);
    char *nameCopy = (char *)malloc(name_len + 1);
    if (nameCopy == NULL)
    {
        fprintf(stderr, "ERROR: Failed to allocate memory for variable name '%s'\n", name);
        return;
    }

    // 手动复制字符串
    memcpy(nameCopy, name, name_len);
    nameCopy[name_len] = '\0';

    // 验证复制是否成功
    if (strlen(nameCopy) != name_len)
    {
        fprintf(stderr, "ERROR: Corrupted name copy\n");
        free(nameCopy);
        return;
    }

    // 分配成功后再赋值
    env->names[env->count] = nameCopy;
    env->values[env->count] = value;
    env->count++;
}
// 在环境中查找变量名
static int findVariable(Environment *env, const char *name)
{
    for (int i = 0; i < env->count; i++)
    {
        if (strcmp(env->names[i], name) == 0)
        {
            return i;
        }
    }
    return -1; // 未找到
}

Value getVariable(Environment *env, Token name)
{
    // 基本参数检查
    if (env == NULL)
    {
        fprintf(stderr, "ERROR: NULL environment in getVariable\n");
        return createNull();
    }

    if (name.lexeme == NULL)
    {
        fprintf(stderr, "ERROR: NULL name.lexeme in getVariable\n");
        return createNull();
    }

    // 验证环境数据结构
    if (env->count < 0 || env->capacity < 0 || env->count > env->capacity)
    {
        fprintf(stderr, "ERROR: Invalid environment state: count=%d, capacity=%d\n",
                env->count, env->capacity);
        return createNull();
    }

    if (env->names == NULL)
    {
        fprintf(stderr, "ERROR: names array is NULL in environment %p\n", (void *)env);
        return createNull();
    }

    if (env->values == NULL)
    {
        fprintf(stderr, "ERROR: values array is NULL in environment %p\n", (void *)env);
        return createNull();
    }

    // 在当前环境中查找变量
    for (int i = 0; i < env->count; i++)
    {

        // 验证当前索引的变量名
        if (env->names[i] == NULL)
        {
            fprintf(stderr, "WARNING: NULL name at index %d in environment %p\n", i, (void *)env);
            continue;
        }

        // 额外安全检查
        if ((uintptr_t)env->names[i] < 0x1000)
        {
            fprintf(stderr, "ERROR: Invalid name pointer %p at index %d\n",
                    (void *)env->names[i], i);
            continue;
        }

        // 特殊处理全局环境中的原生函数
        if (env->enclosing == NULL && i < 3)
        {
            // 快速检查名称的首字母
            if ((name.lexeme[0] == 'p' && strcmp(name.lexeme, "print") == 0) ||
                (name.lexeme[0] == 'c' && strcmp(name.lexeme, "clock") == 0) ||
                (name.lexeme[0] == 't' && strcmp(name.lexeme, "type") == 0))
            {

                if ((i == 0 && name.lexeme[0] == 'p') ||
                    (i == 1 && name.lexeme[0] == 'c') ||
                    (i == 2 && name.lexeme[0] == 't'))
                {
                    return copyValue(env->values[i]);
                }
            }
        }
        if (env->names[i] != NULL && (uintptr_t)env->names[i] >= 0x1000 &&
            name.lexeme != NULL && strcmp(env->names[i], name.lexeme) == 0)
        {
            return copyValue(env->values[i]);
        }
    }

    // 在外层环境中查找
    if (env->enclosing != NULL)
    {
        if (env->enclosing == env)
        { // 检测循环引用
            return createNull();
        }
        return getVariable(env->enclosing, name);
    }

    // 未找到变量
    fprintf(stderr, "ERROR: Undefined variable '%s'\n", name.lexeme);
    return createNull();
}

// 为变量赋值
void assignVariable(Environment *env, Token name, Value value)
{
    int index = findVariable(env, name.lexeme);

    if (index != -1)
    {
        // 释放旧值
        freeValue(env->values[index]);
        // 设置新值
        env->values[index] = copyValue(value);
        return;
    }

    // 在外层环境中查找
    if (env->enclosing != NULL)
    {
        assignVariable(env->enclosing, name, value);
        return;
    }

    // 如果变量不存在，报错
    fprintf(stderr, "未定义的变量 '%s'\n", name.lexeme);
}

// 释放环境
void freeEnvironment(Environment *env)
{
    if (env == NULL)
    {
        return;
    }

    // 验证环境状态的合理性
    if (env->count < 0 || env->capacity < 0 || env->count > env->capacity)
    {
        fprintf(stderr, "WARNING: Environment has invalid state: count=%d, capacity=%d\n",
                env->count, env->capacity);
        env->count = 0; // 防止进入下面的循环
    }

    // 先释放所有值（因为值可能引用了名称）
    if (env->values != NULL)
    {

        for (int i = 0; i < env->count; i++)
        {
            freeValue(env->values[i]); // 这会释放函数对象但不负责释放函数名
        }

        free(env->values);
        env->values = NULL;
    }

    // 然后释放所有变量名
    if (env->names != NULL)
    {
        // 这里添加一个检查，如果环境是全局环境，处理方式可能不同
        // 通常可以通过检查env->enclosing是否为NULL来确定是否为全局环境
        bool isGlobalEnv = (env->enclosing == NULL);

        for (int i = 0; i < env->count; i++)
        {
            if (env->names[i] != NULL)
            {
                // 对于全局环境中的原生函数名，我们可以跳过释放
                // 因为它们可能已经在freeValue中被处理
                if (isGlobalEnv && i < 3)
                { // 假设前3个是原生函数
                }
                else
                {
                    free(env->names[i]);
                }
                env->names[i] = NULL;
            }
        }
        free(env->names);
        env->names = NULL;
    }

    // 重置环境状态
    env->count = 0;
    env->capacity = 0;
    env->enclosing = NULL;
}