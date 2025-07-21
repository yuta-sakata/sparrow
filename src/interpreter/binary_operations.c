#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/interpreter.h"

// 前向声明
static Value handleAddition(Value left, Value right, Interpreter *interpreter);
static Value handleSubtraction(Value left, Value right, Interpreter *interpreter);
static Value handleMultiplication(Value left, Value right, Interpreter *interpreter);
static Value handleDivision(Value left, Value right, Interpreter *interpreter);
static Value handleModulo(Value left, Value right, Interpreter *interpreter);
static Value handleComparison(Value left, Value right, TokenType op, Interpreter *interpreter);
static Value handleEquality(Value left, Value right, TokenType op, Interpreter *interpreter);
static Value handleLogical(Value left, Value right, TokenType op, Interpreter *interpreter);
static Value handleInOperator(Value left, Value right, Interpreter *interpreter);

Value evaluateBinary(Interpreter *interpreter, Expr *expr)
{
    Value left = evaluate(interpreter, expr->as.binary.left);

    // 短路求值处理
    if ((expr->as.binary.op == TOKEN_AND || expr->as.binary.op == TOKEN_OR) &&
        interpreter->hadError == false)
    {
        bool leftTruthy = left.type != VAL_NULL &&
                          !(left.type == VAL_BOOL && !left.as.boolean);

        if ((expr->as.binary.op == TOKEN_AND && !leftTruthy) ||
            (expr->as.binary.op == TOKEN_OR && leftTruthy))
        {
            return left;
        }

        freeValue(left);
        return evaluate(interpreter, expr->as.binary.right);
    }

    Value right = evaluate(interpreter, expr->as.binary.right);

    if (interpreter->hadError)
    {
        freeValue(left);
        freeValue(right);
        return createNull();
    }

    switch (expr->as.binary.op)
    {
    case TOKEN_PLUS:
        return handleAddition(left, right, interpreter);
    case TOKEN_MINUS:
        return handleSubtraction(left, right, interpreter);
    case TOKEN_MULTIPLY:
        return handleMultiplication(left, right, interpreter);
    case TOKEN_DIVIDE:
        return handleDivision(left, right, interpreter);
    case TOKEN_MODULO:
        return handleModulo(left, right, interpreter);
    case TOKEN_LT:
    case TOKEN_LE:
    case TOKEN_GT:
    case TOKEN_GE:
        return handleComparison(left, right, expr->as.binary.op, interpreter);
    case TOKEN_EQ:
    case TOKEN_NE:
        return handleEquality(left, right, expr->as.binary.op, interpreter);
    case TOKEN_AND:
    case TOKEN_OR:
        return handleLogical(left, right, expr->as.binary.op, interpreter);
    case TOKEN_IN:
        return handleInOperator(left, right, interpreter);
    }

    freeValue(left);
    freeValue(right);
    runtimeError(interpreter, "不支持的二元运算符");
    return createNull();
}

static Value handleAddition(Value left, Value right, Interpreter *interpreter)
{
    if (left.type == VAL_NUMBER && right.type == VAL_NUMBER)
    {
        double result = left.as.number + right.as.number;
        freeValue(left);
        freeValue(right);
        return createNumber(result);
    }
    else if (left.type == VAL_STRING && right.type == VAL_STRING)
    {
        // 字符串连接
        int leftLen = strlen(left.as.string);
        int rightLen = strlen(right.as.string);
        char *result = malloc(leftLen + rightLen + 1);

        if (result == NULL)
        {
            freeValue(left);
            freeValue(right);
            runtimeError(interpreter, "内存分配失败");
            return createNull();
        }

        strcpy(result, left.as.string);
        strcat(result, right.as.string);

        freeValue(left);
        freeValue(right);

        Value strValue = createString(result);
        free(result);
        return strValue;
    }
    else if (left.type == VAL_STRING && right.type == VAL_NUMBER)
    {
        // 字符串 + 数字：将数字转换为字符串后连接
        char numberStr[64];
        if (right.as.number == (int)right.as.number)
        {
            snprintf(numberStr, sizeof(numberStr), "%d", (int)right.as.number);
        }
        else
        {
            snprintf(numberStr, sizeof(numberStr), "%g", right.as.number);
        }

        int leftLen = strlen(left.as.string);
        int rightLen = strlen(numberStr);
        char *result = malloc(leftLen + rightLen + 1);

        if (result == NULL)
        {
            freeValue(left);
            freeValue(right);
            runtimeError(interpreter, "内存分配失败");
            return createNull();
        }

        strcpy(result, left.as.string);
        strcat(result, numberStr);

        freeValue(left);
        freeValue(right);

        Value strValue = createString(result);
        free(result);
        return strValue;
    }
    else if (left.type == VAL_NUMBER && right.type == VAL_STRING)
    {
        // 数字 + 字符串：将数字转换为字符串后连接
        char numberStr[64];
        if (left.as.number == (int)left.as.number)
        {
            snprintf(numberStr, sizeof(numberStr), "%d", (int)left.as.number);
        }
        else
        {
            snprintf(numberStr, sizeof(numberStr), "%g", left.as.number);
        }

        int leftLen = strlen(numberStr);
        int rightLen = strlen(right.as.string);
        char *result = malloc(leftLen + rightLen + 1);

        if (result == NULL)
        {
            freeValue(left);
            freeValue(right);
            runtimeError(interpreter, "内存分配失败");
            return createNull();
        }

        strcpy(result, numberStr);
        strcat(result, right.as.string);

        freeValue(left);
        freeValue(right);

        Value strValue = createString(result);
        free(result);
        return strValue;
    }

    freeValue(left);
    freeValue(right);
    runtimeError(interpreter, "+ 运算符只支持数字加法或字符串连接");
    return createNull();
}

static Value handleSubtraction(Value left, Value right, Interpreter *interpreter)
{
    if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "- 运算符的操作数必须是数字。");
        return createNull();
    }

    double result = left.as.number - right.as.number;
    freeValue(left);
    freeValue(right);
    return createNumber(result);
}

static Value handleMultiplication(Value left, Value right, Interpreter *interpreter)
{
    if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "* 运算符的操作数必须是数字。");
        return createNull();
    }

    double result = left.as.number * right.as.number;
    freeValue(left);
    freeValue(right);
    return createNumber(result);
}

static Value handleDivision(Value left, Value right, Interpreter *interpreter)
{
    if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "/ 运算符的操作数必须是数字。");
        return createNull();
    }

    // 检查除数是否为零
    if (right.as.number == 0)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "除数不能为零。");
        return createNull();
    }

    double result = left.as.number / right.as.number;
    freeValue(left);
    freeValue(right);
    return createNumber(result);
}

static Value handleModulo(Value left, Value right, Interpreter *interpreter)
{
    if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "%% 运算符的操作数必须是数字。");
        return createNull();
    }

    // 检查除数是否为零
    if (right.as.number == 0)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "取模运算的除数不能为零。");
        return createNull();
    }

    double result = fmod(left.as.number, right.as.number);
    freeValue(left);
    freeValue(right);
    return createNumber(result);
}

static Value handleComparison(Value left, Value right, TokenType op, Interpreter *interpreter)
{
    if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "比较运算符的操作数必须是数字。");
        return createNull();
    }

    bool result;
    switch (op)
    {
    case TOKEN_LT:
        result = left.as.number < right.as.number;
        break;
    case TOKEN_LE:
        result = left.as.number <= right.as.number;
        break;
    case TOKEN_GT:
        result = left.as.number > right.as.number;
        break;
    case TOKEN_GE:
        result = left.as.number >= right.as.number;
        break;
    default:
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "未知的比较运算符");
        return createNull();
    }

    freeValue(left);
    freeValue(right);
    return createBool(result);
}

static Value handleEquality(Value left, Value right, TokenType op, Interpreter *interpreter)
{
    bool result;

    if (op == TOKEN_EQ)
    {
        result = valuesEqual(left, right);
    }
    else if (op == TOKEN_NE)
    {
        result = !valuesEqual(left, right);
    }
    else
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "未知的相等性运算符");
        return createNull();
    }

    freeValue(left);
    freeValue(right);
    return createBool(result);
}

static Value handleLogical(Value left, Value right, TokenType op, Interpreter *interpreter)
{
    bool result;

    if (op == TOKEN_AND)
    {
        // 逻辑与运算符 (&&)
        if (left.type == VAL_BOOL && !left.as.boolean)
        {
            result = false;
        }
        else if (left.type == VAL_NULL)
        {
            result = false;
        }
        else
        {
            // 左操作数为真，返回右操作数的真值
            result = (right.type != VAL_NULL) &&
                     !(right.type == VAL_BOOL && !right.as.boolean);
        }
    }
    else if (op == TOKEN_OR)
    {
        // 逻辑或运算符 (||)
        bool leftTruthy = (left.type != VAL_NULL) &&
                          !(left.type == VAL_BOOL && !left.as.boolean);

        if (leftTruthy)
        {
            result = true;
        }
        else
        {
            // 左操作数为假，返回右操作数的真值
            result = (right.type != VAL_NULL) &&
                     !(right.type == VAL_BOOL && !right.as.boolean);
        }
    }
    else
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "未知的逻辑运算符");
        return createNull();
    }

    freeValue(left);
    freeValue(right);
    return createBool(result);
}

static Value handleInOperator(Value left, Value right, Interpreter *interpreter)
{
    if (right.type == VAL_ARRAY)
    {
        if (right.as.array == NULL)
        {
            freeValue(left);
            freeValue(right);
            return createBool(false);
        }

        // 在数组中查找元素
        Array *array = right.as.array;
        for (int i = 0; i < array->count; i++)
        {
            if (valuesEqual(left, array->elements[i]))
            {
                freeValue(left);
                freeValue(right);
                return createBool(true);
            }
        }

        freeValue(left);
        freeValue(right);
        return createBool(false);
    }
    else if (right.type == VAL_STRING)
    {
        // 在字符串中查找子字符串
        if (left.type != VAL_STRING)
        {
            freeValue(left);
            freeValue(right);
            runtimeError(interpreter, "当右操作数是字符串时，左操作数也必须是字符串");
            return createNull();
        }

        if (left.as.string == NULL || right.as.string == NULL)
        {
            freeValue(left);
            freeValue(right);
            return createBool(false);
        }

        bool found = strstr(right.as.string, left.as.string) != NULL;
        freeValue(left);
        freeValue(right);
        return createBool(found);
    }
    else
    {
        freeValue(left);
        freeValue(right);
        runtimeError(interpreter, "in 操作符的右操作数必须是数组或字符串");
        return createNull();
    }
}