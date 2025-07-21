#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

Value evaluateUnary(Interpreter *interpreter, Expr *expr) {
    Value right = evaluate(interpreter, expr->as.unary.right);

    switch (expr->as.unary.op) {
    case TOKEN_MINUS:
        if (right.type != VAL_NUMBER) {
            freeValue(right);
            runtimeError(interpreter, "操作数必须是数字。");
            return createNull();
        }
        {
            double value = -right.as.number;
            freeValue(right);
            return createNumber(value);
        }

    case TOKEN_PLUS:
        if (right.type != VAL_NUMBER) {
            freeValue(right);
            runtimeError(interpreter, "操作数必须是数字。");
            return createNull();
        }
        return right;

    case TOKEN_NOT: {
        bool isTruthy = (right.type != VAL_NULL &&
                         !(right.type == VAL_BOOL && !right.as.boolean));
        freeValue(right);
        return createBool(!isTruthy);
    }
    default:
        freeValue(right);
        runtimeError(interpreter, "未知的一元运算符。");
        return createNull();
    }
}

Value evaluatePostfix(Interpreter *interpreter, Expr *expr) {
    if (expr->as.postfix.operand->type != EXPR_VARIABLE) {
        runtimeError(interpreter, "后缀运算符只能应用于变量。");
        return createNull();
    }

    Token varName = expr->as.postfix.operand->as.variable.name;
    Value oldValue = getVariable(interpreter->environment, varName);

    if (interpreter->hadError) {
        return createNull();
    }

    if (oldValue.type != VAL_NUMBER) {
        freeValue(oldValue);
        runtimeError(interpreter, "后缀运算符只能应用于数字类型。");
        return createNull();
    }

    Value newValue;
    if (expr->as.postfix.op == TOKEN_PLUS_PLUS) {
        newValue = createNumber(oldValue.as.number + 1);
    } else if (expr->as.postfix.op == TOKEN_MINUS_MINUS) {
        newValue = createNumber(oldValue.as.number - 1);
    } else {
        freeValue(oldValue);
        runtimeError(interpreter, "未知的后缀运算符。");
        return createNull();
    }

    assignVariable(interpreter->environment, varName, newValue);
    freeValue(newValue);

    return oldValue;
}

Value evaluatePrefix(Interpreter *interpreter, Expr *expr) {
    Token varName = expr->as.prefix.operand->as.variable.name;
    Value oldValue = getVariable(interpreter->environment, varName);

    if (interpreter->hadError) {
        return createNull();
    }

    if (oldValue.type != VAL_NUMBER) {
        freeValue(oldValue);
        runtimeError(interpreter, "前缀运算符只能应用于数字类型。");
        return createNull();
    }

    Value newValue;
    if (expr->as.prefix.op == TOKEN_PLUS_PLUS) {
        newValue = createNumber(oldValue.as.number + 1);
    } else if (expr->as.prefix.op == TOKEN_MINUS_MINUS) {
        newValue = createNumber(oldValue.as.number - 1);
    } else {
        freeValue(oldValue);
        runtimeError(interpreter, "未知的前缀运算符。");
        return createNull();
    }

    assignVariable(interpreter->environment, varName, newValue);
    freeValue(oldValue);
    return newValue;
}