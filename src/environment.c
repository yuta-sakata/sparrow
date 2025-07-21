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
    bool hadError;     // 是否发生错误
    bool hadReturn;    // 是否发生返回
    Value returnValue; // 返回值
} RuntimeError;

// 静态变量用于跟踪运行时状态
static RuntimeError error;

/**
 * 初始化环境结构体
 *
 * 该函数用于初始化一个环境(Environment)结构体，为变量存储分配内存空间。
 * 环境用于存储变量名、值和常量标记，支持嵌套作用域。
 *
 * @param env 指向要初始化的环境结构体的指针
 * @param enclosing 指向外层环境的指针，用于实现作用域嵌套，可以为NULL
 *
 * @note 函数会为环境分配初始容量为8的存储空间
 * @note 如果内存分配失败，程序会打印错误信息并退出
 * @note 所有分配的内存在失败时会被正确释放以避免内存泄漏
 * @note 初始化后所有变量名指针为NULL，常量标记为false
 */
void initEnvironment(Environment *env, Environment *enclosing)
{
    env->enclosing = enclosing;
    env->capacity = 8;
    env->count = 0;

    // 初始化名称和值数组
    env->names = (char **)malloc(sizeof(char *) * env->capacity);
    if (env->names == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }

    env->values = (Value *)malloc(sizeof(Value) * env->capacity);
    if (env->values == NULL)
    {
        free(env->names);
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }

    // 初始化常量标记数组
    env->isConst = (bool *)malloc(sizeof(bool) * env->capacity);
    if (env->isConst == NULL)
    {
        free(env->names);
        free(env->values);
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }

    // 初始化为NULL/false，避免后续比较出错
    for (int i = 0; i < env->capacity; i++)
    {
        env->names[i] = NULL;
        env->isConst[i] = false;
    }
}

/**
 * @brief 在环境中定义一个新变量
 * @details 在指定的环境中定义一个新的变量，包括变量名和值的深度拷贝。
 *          如果环境容量不足，会自动扩展数组大小。新定义的变量默认为非常量。
 *
 * @param env 目标环境指针，用于存储变量
 * @param name 变量名字符串，函数会创建该字符串的副本
 * @param value 变量值，函数会创建该值的深度拷贝
 *
 * @note 函数会处理以下情况：
 *       - 参数验证：检查env和name是否为NULL
 *       - 自动扩容：当环境数组空间不足时，容量翻倍
 *       - 内存管理：对变量名和值进行深度拷贝
 *       - 错误处理：内存分配失败时输出错误信息并返回
 *
 * @warning 如果内存分配失败，函数会输出错误信息到stderr并提前返回
 *          调用者应确保传入的环境结构体已正确初始化
 */
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
        bool *newIsConst = (bool *)realloc(env->isConst, sizeof(bool) * newCapacity);

        if (newNames == NULL || newValues == NULL || newIsConst == NULL)
        {
            fprintf(stderr, "ERROR: Failed to expand environment arrays\n");
            if (newNames != NULL)
                free(newNames);
            if (newValues != NULL)
                free(newValues);
            if (newIsConst != NULL)
                free(newIsConst);
            return;
        }

        env->names = newNames;
        env->values = newValues;
        env->isConst = newIsConst;
        env->capacity = newCapacity;

        // 初始化新分配的空间
        for (int i = env->count; i < env->capacity; i++)
        {
            env->names[i] = NULL;
            env->isConst[i] = false;
        }
    }

    // 先进行深度拷贝，然后检查结果
    size_t name_len = strlen(name);
    char *nameCopy = (char *)malloc(name_len + 1);
    if (nameCopy == NULL)
    {
        fprintf(stderr, "ERROR: Failed to allocate memory for variable name\n");
        return;
    }
    strcpy(nameCopy, name);

    // 设置变量信息
    env->names[env->count] = nameCopy;
    env->values[env->count] = copyValue(value);
    env->isConst[env->count] = false; // 普通变量
    env->count++;
}

/**
 * 在环境中定义一个常量
 *
 * 该函数向指定的环境中添加一个新的常量定义。常量一旦定义后不能被修改。
 * 如果环境容量不足，会自动扩展数组大小。
 *
 * @param env 指向环境结构体的指针，不能为NULL
 * @param name 常量名称，不能为NULL，会进行深拷贝存储
 * @param value 常量值，会进行深拷贝存储
 *
 * @note 该函数会：
 *       - 检查参数有效性
 *       - 在需要时自动扩展环境容量（容量翻倍）
 *       - 对常量名称和值进行深拷贝
 *       - 将常量标记为不可修改
 *
 * @warning 如果内存分配失败或参数为NULL，函数会打印错误信息并提前返回
 */
void defineConstant(Environment *env, const char *name, Value value)
{
    if (env == NULL || name == NULL)
    {
        fprintf(stderr, "ERROR: NULL parameter in defineConstant\n");
        return;
    }

    // 确保数组有足够空间
    if (env->count >= env->capacity)
    {
        int newCapacity = env->capacity * 2;
        char **newNames = (char **)realloc(env->names, sizeof(char *) * newCapacity);
        Value *newValues = (Value *)realloc(env->values, sizeof(Value) * newCapacity);
        bool *newIsConst = (bool *)realloc(env->isConst, sizeof(bool) * newCapacity);

        if (newNames == NULL || newValues == NULL || newIsConst == NULL)
        {
            fprintf(stderr, "ERROR: Failed to expand environment arrays\n");
            if (newNames != NULL)
                free(newNames);
            if (newValues != NULL)
                free(newValues);
            if (newIsConst != NULL)
                free(newIsConst);
            return;
        }

        env->names = newNames;
        env->values = newValues;
        env->isConst = newIsConst;
        env->capacity = newCapacity;

        // 初始化新分配的空间
        for (int i = env->count; i < env->capacity; i++)
        {
            env->names[i] = NULL;
            env->isConst[i] = false;
        }
    }

    // 先进行深度拷贝，然后检查结果
    size_t name_len = strlen(name);
    char *nameCopy = (char *)malloc(name_len + 1);
    if (nameCopy == NULL)
    {
        fprintf(stderr, "ERROR: Failed to allocate memory for constant name\n");
        return;
    }
    strcpy(nameCopy, name);

    // 设置常量信息
    env->names[env->count] = nameCopy;
    env->values[env->count] = copyValue(value);
    env->isConst[env->count] = true; // 标记为常量
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

/**
 * 从环境中获取指定名称的变量值
 *
 * 该函数在给定的环境及其外层环境中递归查找指定名称的变量。
 * 包含完整的参数验证和错误处理机制。
 *
 * @param env 要搜索的环境指针，不能为NULL
 * @param name 要查找的变量名Token，其lexeme字段不能为NULL
 *
 * @return Value 返回找到的变量值的副本，如果未找到或发生错误则返回NULL值
 *
 * @note 函数特性：
 *       - 执行完整的参数和环境状态验证
 *       - 对全局环境中的原生函数(print, clock, type)进行优化处理
 *       - 递归搜索外层环境直到找到变量或到达全局环境
 *       - 检测并防止环境循环引用
 *       - 返回变量值的副本而非原始引用
 *
 * @warning 错误情况：
 *          - 环境指针为NULL
 *          - 变量名为NULL
 *          - 环境数据结构损坏
 *          - 检测到环境循环引用
 *          - 变量未定义
 */
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
        if (env->enclosing == NULL && i < 5)
        {
            // 按照实际注册顺序：print(0), println(1), clock(2), type(3), input(4)
            if ((name.lexeme[0] == 'p' && strcmp(name.lexeme, "print") == 0 && i == 0) ||
                (name.lexeme[0] == 'p' && strcmp(name.lexeme, "println") == 0 && i == 1) ||
                (name.lexeme[0] == 'c' && strcmp(name.lexeme, "clock") == 0 && i == 2) ||
                (name.lexeme[0] == 't' && strcmp(name.lexeme, "type") == 0 && i == 3) ||
                (name.lexeme[0] == 'i' && strcmp(name.lexeme, "input") == 0 && i == 4))
            {
                return copyValue(env->values[i]);
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

/**
 * 为环境中的变量赋值
 *
 * 在当前环境及其外层环境中查找指定的变量并为其赋值。
 * 如果变量是常量，则拒绝赋值操作。
 * 如果变量不存在，则输出错误信息。
 *
 * @param env 目标环境指针
 * @param name 变量名称的token
 * @param value 要赋给变量的新值
 *
 * @note 函数会自动释放变量的旧值并复制新值
 * @note 如果变量在当前环境中不存在，会递归查找外层环境
 * @note 对常量赋值或对未定义变量赋值会输出错误信息到stderr
 */
void assignVariable(Environment *env, Token name, Value value)
{
    int index = findVariable(env, name.lexeme);

    if (index != -1)
    {
        // 检查是否为常量
        if (env->isConst[index])
        {
            fprintf(stderr, "错误：不能对常量 '%s' 赋值\n", name.lexeme);
            return;
        }

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

/**
 * 释放环境对象及其所有资源
 *
 * 该函数负责完全清理一个环境对象，包括释放所有动态分配的内存和重置状态。
 * 释放过程包括：
 * 1. 验证环境状态的合理性，如果发现异常状态会发出警告并修正
 * 2. 释放所有存储的值对象
 * 3. 释放变量名数组（对于全局环境会跳过前3个原生函数名）
 * 4. 释放常量标记数组
 * 5. 重置环境的所有字段为安全状态
 *
 * @param env 要释放的环境对象指针，如果为NULL则函数直接返回
 *
 * @note 该函数不会释放env指针本身，只释放其内部资源
 * @note 对于全局环境，前3个变量名被视为原生函数名，不会被释放
 * @warning 调用此函数后，env对象的内部状态将被重置，但对象本身仍然存在
 */
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
        env->count = 0;
    }

    // 先释放所有值
    if (env->values != NULL)
    {
        for (int i = 0; i < env->count; i++)
        {
            freeValue(env->values[i]);
        }
        free(env->values);
        env->values = NULL;
    }

    // 释放所有变量名
    if (env->names != NULL)
    {
        bool isGlobalEnv = (env->enclosing == NULL);
        for (int i = 0; i < env->count; i++)
        {
            if (env->names[i] != NULL)
            {
                if (isGlobalEnv && i < 3)
                {
                    // 跳过全局环境中的原生函数名
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

    // 释放常量标记数组
    if (env->isConst != NULL)
    {
        free(env->isConst);
        env->isConst = NULL;
    }

    // 重置环境状态
    env->count = 0;
    env->capacity = 0;
    env->enclosing = NULL;
}

/**
 * 获取变量的引用
 *
 * 在指定环境及其外层环境中查找变量，并返回该变量值的引用。
 * 查找顺序为从当前环境开始，逐层向外层环境查找，直到找到匹配的变量名或遍历完所有环境。
 *
 * @param env 要查找的环境指针，如果为NULL则直接返回NULL
 * @param name 要查找的变量名，如果为NULL则直接返回NULL
 * @return Value* 如果找到变量则返回指向该变量值的指针引用，否则返回NULL
 *
 * @note 返回的是变量值的引用，可以用于修改变量的值
 * @note 函数会递归查找外层环境，直到找到变量或到达最外层环境
 */
Value *getVariableRef(Environment *env, const char *name)
{
    if (env == NULL || name == NULL)
        return NULL;

    // 在当前环境中查找
    for (int i = 0; i < env->count; i++)
    {
        if (env->names[i] != NULL && strcmp(env->names[i], name) == 0)
        {
            return &env->values[i]; // 返回引用
        }
    }

    // 在外层环境中查找
    if (env->enclosing != NULL)
    {
        return getVariableRef(env->enclosing, name);
    }

    return NULL; // 未找到
}

void initStaticStorage(StaticStorage *storage) {
    storage->capacity = 8;
    storage->count = 0;
    
    storage->names = (char **)malloc(sizeof(char *) * storage->capacity);
    storage->values = (Value *)malloc(sizeof(Value) * storage->capacity);
    storage->isConst = (bool *)malloc(sizeof(bool) * storage->capacity);
    
    if (storage->names == NULL || storage->values == NULL || storage->isConst == NULL) {
        fprintf(stderr, "静态存储内存分配失败\n");
        exit(1);
    }
    
    for (int i = 0; i < storage->capacity; i++) {
        storage->names[i] = NULL;
        storage->isConst[i] = false;
    }
}
void defineStaticVariable(StaticStorage *storage, const char *name, Value value, bool isConst) {
    if (storage == NULL || name == NULL) {
        fprintf(stderr, "ERROR: NULL parameter in defineStaticVariable\n");
        return;
    }
    
    // 检查是否需要扩容
    if (storage->count >= storage->capacity) {
        int newCapacity = storage->capacity * 2;
        char **newNames = (char **)realloc(storage->names, sizeof(char *) * newCapacity);
        Value *newValues = (Value *)realloc(storage->values, sizeof(Value) * newCapacity);
        bool *newIsConst = (bool *)realloc(storage->isConst, sizeof(bool) * newCapacity);
        
        if (newNames == NULL || newValues == NULL || newIsConst == NULL) {
            fprintf(stderr, "ERROR: Failed to expand static storage\n");
            return;
        }
        
        storage->names = newNames;
        storage->values = newValues;
        storage->isConst = newIsConst;
        storage->capacity = newCapacity;
        
        for (int i = storage->count; i < storage->capacity; i++) {
            storage->names[i] = NULL;
            storage->isConst[i] = false;
        }
    }
    
    // 复制名称
    size_t nameLen = strlen(name);
    char *nameCopy = (char *)malloc(nameLen + 1);
    if (nameCopy == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for static variable name\n");
        return;
    }
    strcpy(nameCopy, name);
    
    storage->names[storage->count] = nameCopy;
    storage->values[storage->count] = copyValue(value);
    storage->isConst[storage->count] = isConst;
    storage->count++;
}

Value getStaticVariable(StaticStorage *storage, const char *name) {
    if (storage == NULL || name == NULL) {
        return createNull();
    }
    
    for (int i = 0; i < storage->count; i++) {
        if (storage->names[i] != NULL && strcmp(storage->names[i], name) == 0) {
            return copyValue(storage->values[i]);
        }
    }
    
    return createNull();
}

void assignStaticVariable(StaticStorage *storage, const char *name, Value value) {
    if (storage == NULL || name == NULL) {
        fprintf(stderr, "ERROR: NULL parameter in assignStaticVariable\n");
        return;
    }
    
    for (int i = 0; i < storage->count; i++) {
        if (storage->names[i] != NULL && strcmp(storage->names[i], name) == 0) {
            if (storage->isConst[i]) {
                fprintf(stderr, "ERROR: Cannot assign to static constant '%s'\n", name);
                return;
            }
            
            freeValue(storage->values[i]);
            storage->values[i] = copyValue(value);
            return;
        }
    }
    
    fprintf(stderr, "ERROR: Undefined static variable '%s'\n", name);
}

void freeStaticStorage(StaticStorage *storage) {
    if (storage == NULL) return;
    
    for (int i = 0; i < storage->count; i++) {
        if (storage->names[i] != NULL) {
            free(storage->names[i]);
        }
        freeValue(storage->values[i]);
    }
    
    free(storage->names);
    free(storage->values);
    free(storage->isConst);
    
    storage->names = NULL;
    storage->values = NULL;
    storage->isConst = NULL;
    storage->count = 0;
    storage->capacity = 0;
}