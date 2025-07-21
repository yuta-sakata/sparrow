#ifndef SPARROW_FUNCTION_CALLS_H
#define SPARROW_FUNCTION_CALLS_H

#include "interpreter_core.h"

// 函数调用函数
Value evaluateCall(Interpreter *interpreter, Expr *expr);
Value callFunction(Interpreter *interpreter, Function *function, Value *arguments, int argCount);

#endif // SPARROW_FUNCTION_CALLS_H