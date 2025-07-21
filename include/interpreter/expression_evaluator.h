#ifndef SPARROW_EXPRESSION_EVALUATOR_H
#define SPARROW_EXPRESSION_EVALUATOR_H

#include "interpreter_core.h"

// 主要求值函数
Value evaluate(Interpreter* interpreter, Expr* expr);

// 基础表达式求值函数
Value evaluateLiteral(Expr *expr);
Value evaluateGrouping(Interpreter *interpreter, Expr *expr);
Value evaluateVariable(Interpreter *interpreter, Expr *expr);
Value evaluateAssign(Interpreter *interpreter, Expr *expr);
Value evaluateDotAccess(Interpreter *interpreter, Expr *expr);
Value evaluateStructLiteral(Interpreter *interpreter, Expr *expr);
Value evaluateStructAssign(Interpreter *interpreter, Expr *expr);

#endif // SPARROW_EXPRESSION_EVALUATOR_H