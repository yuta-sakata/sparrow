#ifndef SPARROW_ENVIRONMENT_H
#define SPARROW_ENVIRONMENT_H

#include "lexer.h"
#include "value.h"

typedef struct Environment Environment;

// 环境结构，用于存储变量
struct Environment {
    Environment* enclosing;  // 外层环境
    int capacity;
    int count;
    char** names;           // 变量名数组
    Value* values;          // 变量值数组
};

// 初始化环境
void initEnvironment(Environment* env, Environment* enclosing);

// 定义变量
void defineVariable(Environment* env, const char* name, Value value);

// 获取变量
Value getVariable(Environment* env, Token name);

// 赋值变量
void assignVariable(Environment* env, Token name, Value value);

// 释放环境
void freeEnvironment(Environment* env);

// 查找变量的索引
Value *getVariableRef(Environment *env, const char *name);

#endif // SPARROW_ENVIRONMENT_H