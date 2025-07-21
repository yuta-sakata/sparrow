// include/interpreter/array_operations.h
#ifndef SPARROW_ARRAY_OPERATIONS_H
#define SPARROW_ARRAY_OPERATIONS_H

#include "interpreter_core.h"

// 数组操作函数
Value evaluateArrayLiteral(Interpreter *interpreter, Expr *expr);
Value evaluateArrayAccess(Interpreter *interpreter, Expr *expr);
Value evaluateArrayAssign(Interpreter *interpreter, Expr *expr);

#endif // SPARROW_ARRAY_OPERATIONS_H