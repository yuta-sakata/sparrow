#ifndef SPARROW_INTERPRETER_CORE_H
#define SPARROW_INTERPRETER_CORE_H

#include "../ast.h"
#include "../environment.h"
#include "../value.h"

typedef struct {
    Environment* globals;   
    Environment* environment; 
    StaticStorage *staticStorage;  
    bool hadError;
    char errorMessage[256];
    bool hasMainFunction;  
    Function* mainFunction; 
} Interpreter;

// 定义全局状态结构体类型
typedef struct {
    bool hasReturn;
    Value value;
} ReturnStatusType;

typedef struct {
    bool hasBreak;
} BreakStatusType;

// 全局状态变量外部声明
extern ReturnStatusType returnStatus;
extern BreakStatusType breakStatus;

// 核心解释器函数
void initInterpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter, Stmt** statements, int count);
void execute(Interpreter* interpreter, Stmt* stmt);
bool hadInterpreterError(Interpreter* interpreter);
const char* getInterpreterError(Interpreter* interpreter);
void freeInterpreter(Interpreter* interpreter);
void runtimeError(Interpreter *interpreter, const char *format, ...);

#endif // SPARROW_INTERPRETER_CORE_H