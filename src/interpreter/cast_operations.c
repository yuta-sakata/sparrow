#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

Value evaluateCast(Interpreter *interpreter, Expr *expr) {
    Value value = evaluate(interpreter, expr->as.cast.expression);
    if (interpreter->hadError) {
        return createNull();
    }

    BaseType targetType = expr->as.cast.targetType;

    switch (targetType) {
    case TYPE_INT:
        if (value.type == VAL_NUMBER) {
            int intValue = (int)value.as.number;
            freeValue(value);
            return createNumber((double)intValue);
        } else if (value.type == VAL_STRING) {
            int intValue = atoi(value.as.string);
            freeValue(value);
            return createNumber((double)intValue);
        }
        break;

    case TYPE_FLOAT:
        if (value.type == VAL_NUMBER) {
            return value;
        } else if (value.type == VAL_STRING) {
            double floatValue = atof(value.as.string);
            freeValue(value);
            return createNumber(floatValue);
        }
        break;

    case TYPE_STRING:
        if (value.type == VAL_NUMBER) {
            char buffer[32];
            if (value.as.number == (int)value.as.number) {
                snprintf(buffer, sizeof(buffer), "%d", (int)value.as.number);
            } else {
                snprintf(buffer, sizeof(buffer), "%g", value.as.number);
            }
            freeValue(value);
            return createString(buffer);
        } else if (value.type == VAL_BOOL) {
            const char *str = value.as.boolean ? "true" : "false";
            freeValue(value);
            return createString(str);
        }
        break;

    case TYPE_BOOL:
        if (value.type == VAL_NUMBER) {
            bool boolValue = value.as.number != 0;
            freeValue(value);
            return createBool(boolValue);
        } else if (value.type == VAL_STRING) {
            bool boolValue = strlen(value.as.string) > 0;
            freeValue(value);
            return createBool(boolValue);
        }
        break;

    default:
        break;
    }

    freeValue(value);
    runtimeError(interpreter, "无法将类型转换为目标类型");
    return createNull();
}