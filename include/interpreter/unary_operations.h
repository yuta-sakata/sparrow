// include/interpreter/unary_operations.h
#ifndef SPARROW_UNARY_OPERATIONS_H
#define SPARROW_UNARY_OPERATIONS_H

#include "interpreter_core.h"

// 一元运算函数
Value evaluateUnary(Interpreter *interpreter, Expr *expr);
Value evaluatePostfix(Interpreter *interpreter, Expr *expr);
Value evaluatePrefix(Interpreter *interpreter, Expr *expr);

#endif // SPARROW_UNARY_OPERATIONS_H