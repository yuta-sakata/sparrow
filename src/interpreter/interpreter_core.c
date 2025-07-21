#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../include/interpreter.h"
#include "../include/native_functions.h"

// 定义全局状态变量（实际定义，不是声明）
ReturnStatusType returnStatus = {false, {0}};
BreakStatusType breakStatus = {false};

void initInterpreter(Interpreter *interpreter) {
    interpreter->globals = (Environment *)malloc(sizeof(Environment));
    initEnvironment(interpreter->globals, NULL);
    interpreter->environment = interpreter->globals;
    interpreter->hadError = false;
    interpreter->errorMessage[0] = '\0';
    interpreter->hasMainFunction = false;
    interpreter->mainFunction = NULL;

    // 初始化静态存储
    interpreter->staticStorage = (StaticStorage *)malloc(sizeof(StaticStorage));
    if (interpreter->staticStorage == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate static storage\n");
        exit(1);
    }
    initStaticStorage(interpreter->staticStorage);

    registerAllNativeFunctions(interpreter);
}

void interpret(Interpreter *interpreter, Stmt **statements, int count) {
    // 第一阶段：执行函数定义和枚举声明
    for (int i = 0; i < count; i++) {
        bool isEnum = (statements[i]->type == STMT_ENUM);
        bool isFunction = (statements[i]->type == STMT_FUNCTION);
        if (isEnum || isFunction) {
            execute(interpreter, statements[i]);
            if (interpreter->hadError) {
                return;
            }
        }
    }

    // 第二阶段：执行其他语句
    for (int i = 0; i < count; i++) {
        bool isEnum = (statements[i]->type == STMT_ENUM);
        bool isFunction = (statements[i]->type == STMT_FUNCTION);

        if (!isEnum && !isFunction) {
            execute(interpreter, statements[i]);
            if (interpreter->hadError) {
                return;
            }
        }
    }

    // 如果找到了 main 函数，自动调用它
    if (interpreter->hasMainFunction && interpreter->mainFunction != NULL) {
        Value *noArgs = NULL;
        Value result = callFunction(interpreter, interpreter->mainFunction, noArgs, 0);
        freeValue(result);
    }
}

void runtimeError(Interpreter *interpreter, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(interpreter->errorMessage, sizeof(interpreter->errorMessage), format, args);
    va_end(args);

    interpreter->hadError = true;
}

bool hadInterpreterError(Interpreter *interpreter) {
    return interpreter != NULL && interpreter->hadError;
}

const char *getInterpreterError(Interpreter *interpreter) {
    if (interpreter == NULL)
        return "解释器未初始化";
    return interpreter->errorMessage;
}

void freeInterpreter(Interpreter *interpreter) {
    if (interpreter == NULL)
        return;

    if (interpreter->globals != NULL) {
        if (interpreter->environment == interpreter->globals) {
            interpreter->environment = NULL;
        }
        freeEnvironment(interpreter->globals);
        free(interpreter->globals);
        interpreter->globals = NULL;
    }

    if (interpreter->staticStorage != NULL) {
        freeStaticStorage(interpreter->staticStorage);
        free(interpreter->staticStorage);
        interpreter->staticStorage = NULL;
    }

    interpreter->environment = NULL;
    interpreter->mainFunction = NULL;
    interpreter->hasMainFunction = false;
    interpreter->hadError = false;
    interpreter->errorMessage[0] = '\0';
}