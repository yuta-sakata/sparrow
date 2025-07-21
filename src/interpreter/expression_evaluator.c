#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/interpreter.h"

Value evaluate(Interpreter *interpreter, Expr *expr) {
    if (expr == NULL) {
        printf("ERROR: Trying to evaluate NULL expression\n");
        return createNull();
    }

    switch (expr->type) {
    case EXPR_LITERAL:
        return evaluateLiteral(expr);
    case EXPR_GROUPING:
        return evaluateGrouping(interpreter, expr);
    case EXPR_UNARY:
        return evaluateUnary(interpreter, expr);
    case EXPR_BINARY:
        return evaluateBinary(interpreter, expr);
    case EXPR_VARIABLE:
        return evaluateVariable(interpreter, expr);
    case EXPR_ASSIGN:
        return evaluateAssign(interpreter, expr);
    case EXPR_CALL:
        return evaluateCall(interpreter, expr);
    case EXPR_POSTFIX:
        return evaluatePostfix(interpreter, expr);
    case EXPR_PREFIX:
        return evaluatePrefix(interpreter, expr);
    case EXPR_ARRAY_LITERAL:
        return evaluateArrayLiteral(interpreter, expr);
    case EXPR_ARRAY_ACCESS:
        return evaluateArrayAccess(interpreter, expr);
    case EXPR_ARRAY_ASSIGN:
        return evaluateArrayAssign(interpreter, expr);
    case EXPR_CAST:
        return evaluateCast(interpreter, expr);
    }

    return createNull();
}

Value evaluateLiteral(Expr *expr) {
    Token token = expr->as.literal.value;

    switch (token.type) {
    case TOKEN_INTEGER: {
        double value = atof(token.lexeme);
        return createNumber(value);
    }
    case TOKEN_FLOAT: {
        double value = atof(token.lexeme);
        return createNumber(value);
    }
    case TOKEN_STRING: {
        int len = strlen(token.lexeme);
        if (len >= 2 && token.lexeme[0] == '"' && token.lexeme[len - 1] == '"') {
            if (token.value.stringValue != NULL) {
                return createString(token.value.stringValue);
            } else {
                char *str = malloc(len - 1);
                if (str != NULL) {
                    strncpy(str, token.lexeme + 1, len - 2);
                    str[len - 2] = '\0';
                    Value result = createString(str);
                    free(str);
                    return result;
                }
            }
        }
        return createString(token.lexeme);
    }
    case TOKEN_TRUE:
        return createBool(true);
    case TOKEN_FALSE:
        return createBool(false);
    case TOKEN_NULL:
        return createNull();
    default:
        return createNull();
    }
}

Value evaluateGrouping(Interpreter *interpreter, Expr *expr) {
    return evaluate(interpreter, expr->as.grouping.expression);
}

Value evaluateVariable(Interpreter *interpreter, Expr *expr) {
    // 首先在静态存储中查找
    Value result = getStaticVariable(interpreter->staticStorage, expr->as.variable.name.lexeme);

    if (result.type != VAL_NULL) {
        return result;
    }

    // 在当前环境中查找
    result = getVariable(interpreter->environment, expr->as.variable.name);

    if (result.type == VAL_NULL && interpreter->hadError) {
        interpreter->hadError = false;
        interpreter->errorMessage[0] = '\0';

        // 构造静态函数名
        char *staticName = malloc(strlen(expr->as.variable.name.lexeme) + 8);
        if (staticName != NULL) {
            sprintf(staticName, "static_%s", expr->as.variable.name.lexeme);
            Token staticToken = expr->as.variable.name;
            staticToken.lexeme = staticName;
            result = getVariable(interpreter->globals, staticToken);
            free(staticName);
        }
    }

    return result;
}

Value evaluateAssign(Interpreter *interpreter, Expr *expr) {
    Value value = evaluate(interpreter, expr->as.assign.value);

    if (interpreter->hadError)
        return createNull();

    // 首先尝试在静态存储中赋值
    bool foundInStatic = false;
    for (int i = 0; i < interpreter->staticStorage->count; i++) {
        if (interpreter->staticStorage->names[i] != NULL &&
            strcmp(interpreter->staticStorage->names[i], expr->as.assign.name.lexeme) == 0) {
            foundInStatic = true;
            assignStaticVariable(interpreter->staticStorage, expr->as.assign.name.lexeme, value);
            break;
        }
    }

    if (!foundInStatic) {
        assignVariable(interpreter->environment, expr->as.assign.name, value);
    }

    return value;
}