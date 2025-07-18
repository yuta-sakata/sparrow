#ifndef SPARROW_INTERPRETER_H
#define SPARROW_INTERPRETER_H

#include "ast.h"
#include "environment.h"
#include "value.h"

typedef struct {
    Environment* globals;   // 全局环境
    Environment* environment; // 当前环境
    StaticStorage *staticStorage;  // 添加静态存储
    bool hadError;
    char errorMessage[256];
    bool hasMainFunction;  // 标记是否找到 main 函数
    Function* mainFunction; //保存 main 函数引用
} Interpreter;

// 初始化解释器
void initInterpreter(Interpreter* interpreter);

// 解释程序
void interpret(Interpreter* interpreter, Stmt** statements, int count);

// 表达式求值
Value evaluate(Interpreter* interpreter, Expr* expr);

// 执行语句
void execute(Interpreter* interpreter, Stmt* stmt);

// 错误处理
bool hadInterpreterError(Interpreter* interpreter);
const char* getInterpreterError(Interpreter* interpreter);

// 释放解释器
void freeInterpreter(Interpreter* interpreter);

#endif // SPARROW_INTERPRETER_H