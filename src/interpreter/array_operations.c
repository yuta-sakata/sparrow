#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

Value evaluateArrayLiteral(Interpreter *interpreter, Expr *expr) {
    Value arrayValue = createArray(TYPE_ANY, expr->as.arrayLiteral.elementCount);
    
    if (arrayValue.type == VAL_NULL) {
        runtimeError(interpreter, "创建数组失败");
        return createNull();
    }

    for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++) {
        Value element = evaluate(interpreter, expr->as.arrayLiteral.elements[i]);
        
        if (interpreter->hadError) {
            freeValue(arrayValue);
            return createNull();
        }

        Value elementCopy = copyValue(element);
        arrayPush(arrayValue.as.array, elementCopy);
        freeValue(element);
    }

    return arrayValue;
}

Value evaluateArrayAccess(Interpreter *interpreter, Expr *expr) {
    Value arrayValue = evaluate(interpreter, expr->as.arrayAccess.array);
    if (interpreter->hadError) {
        return createNull();
    }

    Value indexValue = evaluate(interpreter, expr->as.arrayAccess.index);
    if (interpreter->hadError) {
        freeValue(arrayValue);
        return createNull();
    }

    if (arrayValue.type != VAL_ARRAY) {
        freeValue(arrayValue);
        freeValue(indexValue);
        runtimeError(interpreter, "只能对数组进行索引访问");
        return createNull();
    }

    if (indexValue.type != VAL_NUMBER) {
        freeValue(arrayValue);
        freeValue(indexValue);
        runtimeError(interpreter, "数组索引必须是数字");
        return createNull();
    }

    int index = (int)indexValue.as.number;
    Value result = arrayGet(arrayValue.as.array, index);

    freeValue(arrayValue);
    freeValue(indexValue);
    return result;
}

Value evaluateArrayAssign(Interpreter *interpreter, Expr *expr) {
    if (expr->as.arrayAssign.array->type == EXPR_VARIABLE) {
        Value *arrayRef = getVariableRef(interpreter->environment,
                                        expr->as.arrayAssign.array->as.variable.name.lexeme);

        if (arrayRef == NULL || arrayRef->type != VAL_ARRAY) {
            runtimeError(interpreter, "只能对数组进行索引赋值");
            return createNull();
        }

        Value indexValue = evaluate(interpreter, expr->as.arrayAssign.index);
        if (interpreter->hadError) {
            return createNull();
        }

        Value value = evaluate(interpreter, expr->as.arrayAssign.value);
        if (interpreter->hadError) {
            freeValue(indexValue);
            return createNull();
        }

        if (indexValue.type != VAL_NUMBER) {
            freeValue(indexValue);
            freeValue(value);
            runtimeError(interpreter, "数组索引必须是数字");
            return createNull();
        }

        int index = (int)indexValue.as.number;
        arraySet(arrayRef->as.array, index, value);

        freeValue(indexValue);
        return value;
    } else {
        Value arrayValue = evaluate(interpreter, expr->as.arrayAssign.array);
        if (interpreter->hadError) return createNull();

        Value indexValue = evaluate(interpreter, expr->as.arrayAssign.index);
        if (interpreter->hadError) {
            freeValue(arrayValue);
            return createNull();
        }

        Value value = evaluate(interpreter, expr->as.arrayAssign.value);
        if (interpreter->hadError) {
            freeValue(arrayValue);
            freeValue(indexValue);
            return createNull();
        }

        if (arrayValue.type != VAL_ARRAY) {
            freeValue(arrayValue);
            freeValue(indexValue);
            freeValue(value);
            runtimeError(interpreter, "只能对数组进行索引赋值");
            return createNull();
        }

        if (indexValue.type != VAL_NUMBER) {
            freeValue(arrayValue);
            freeValue(indexValue);
            freeValue(value);
            runtimeError(interpreter, "数组索引必须是数字");
            return createNull();
        }

        int index = (int)indexValue.as.number;
        arraySet(arrayValue.as.array, index, value);

        freeValue(arrayValue);
        freeValue(indexValue);
        return value;
    }
}