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
    case EXPR_DOT_ACCESS:
        return evaluateDotAccess(interpreter, expr);
    case EXPR_STRUCT_LITERAL:
        return evaluateStructLiteral(interpreter, expr);
    case EXPR_STRUCT_ASSIGN:
        return evaluateStructAssign(interpreter, expr);
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

Value evaluateDotAccess(Interpreter *interpreter, Expr *expr) {
    Expr *object = expr->as.dotAccess.object;
    Token member = expr->as.dotAccess.member;
    
    // 首先求值对象表达式
    Value objectValue = evaluate(interpreter, object);
    if (interpreter->hadError) {
        return createNull();
    }
    
    // 检查对象类型
    if (objectValue.type == VAL_STRUCT) {
        // 结构体成员访问
        StructValue *structValue = objectValue.as.structValue;
        const char *memberName = member.lexeme;
        
        // 查找对应的字段
        for (int i = 0; i < structValue->fieldCount; i++) {
            if (strcmp(structValue->fields[i].name, memberName) == 0) {
                Value result = copyValue(*structValue->fields[i].value);
                freeValue(objectValue);
                return result;
            }
        }
        
        // 字段未找到
        freeValue(objectValue);
        runtimeError(interpreter, "Struct field not found");
        return createNull();
    }
    else if (object->type == EXPR_VARIABLE) {
        // 枚举成员访问（保持原有逻辑）
        freeValue(objectValue); // 释放不需要的值
        
        const char *enumName = object->as.variable.name.lexeme;
        const char *memberName = member.lexeme;
        
        // 分配内存来存储完整名称
        size_t fullNameLen = strlen(enumName) + strlen(memberName) + 2; // +2 for '_' and '\0'
        char *fullName = malloc(fullNameLen);
        if (fullName == NULL) {
            runtimeError(interpreter, "Memory allocation failed");
            return createNull();
        }
        
        snprintf(fullName, fullNameLen, "%s_%s", enumName, memberName);
        
        // 查找枚举成员值
        Token enumMemberToken;
        enumMemberToken.lexeme = fullName;
        enumMemberToken.type = TOKEN_IDENTIFIER;
        
        Value result = getVariable(interpreter->globals, enumMemberToken);
        
        free(fullName);
        return result;
    }
    else {
        freeValue(objectValue);
        runtimeError(interpreter, "Can only access members of structs and enums");
        return createNull();
    }
}

Value evaluateStructLiteral(Interpreter *interpreter, Expr *expr) {
    const char *structName = expr->as.structLiteral.structName.lexeme;
    int fieldCount = expr->as.structLiteral.fieldCount;
    StructFieldInit *fieldInits = expr->as.structLiteral.fields;
    
    // 分配结构体字段值数组
    StructFieldValue *fields = malloc(sizeof(StructFieldValue) * fieldCount);
    if (fields == NULL) {
        runtimeError(interpreter, "Memory allocation failed");
        return createNull();
    }
    
    // 求值每个字段
    for (int i = 0; i < fieldCount; i++) {
        // 复制字段名
        size_t nameLen = strlen(fieldInits[i].name.lexeme);
        fields[i].name = malloc(nameLen + 1);
        if (fields[i].name == NULL) {
            // 清理已分配的内存
            for (int j = 0; j < i; j++) {
                free(fields[j].name);
                freeValue(*fields[j].value);
                free(fields[j].value);
            }
            free(fields);
            runtimeError(interpreter, "Memory allocation failed");
            return createNull();
        }
        strcpy(fields[i].name, fieldInits[i].name.lexeme);
        
        // 求值字段值
        Value fieldValue = evaluate(interpreter, fieldInits[i].value);
        if (interpreter->hadError) {
            // 清理已分配的内存
            for (int j = 0; j <= i; j++) {
                free(fields[j].name);
                if (j < i) {
                    freeValue(*fields[j].value);
                    free(fields[j].value);
                }
            }
            free(fields);
            return createNull();
        }
        
        // 分配并复制值
        fields[i].value = malloc(sizeof(Value));
        if (fields[i].value == NULL) {
            // 清理已分配的内存
            freeValue(fieldValue);
            for (int j = 0; j <= i; j++) {
                free(fields[j].name);
                if (j < i) {
                    freeValue(*fields[j].value);
                    free(fields[j].value);
                }
            }
            free(fields);
            runtimeError(interpreter, "Memory allocation failed");
            return createNull();
        }
        *fields[i].value = fieldValue;
    }
    
    return createStruct(structName, fields, fieldCount);
}

Value evaluateStructAssign(Interpreter *interpreter, Expr *expr) {
    // 首先求值要赋的值
    Value value = evaluate(interpreter, expr->as.structAssign.value);
    if (interpreter->hadError) {
        return createNull();
    }
    
    // 求值结构体对象
    Value objectValue = evaluate(interpreter, expr->as.structAssign.object);
    if (interpreter->hadError) {
        freeValue(value);
        return createNull();
    }
    
    // 检查对象是否为结构体
    if (objectValue.type != VAL_STRUCT) {
        freeValue(value);
        freeValue(objectValue);
        runtimeError(interpreter, "Can only assign to struct fields");
        return createNull();
    }
    
    // 查找并更新字段
    StructValue *structValue = objectValue.as.structValue;
    const char *fieldName = expr->as.structAssign.field.lexeme;
    
    for (int i = 0; i < structValue->fieldCount; i++) {
        if (strcmp(structValue->fields[i].name, fieldName) == 0) {
            // 释放旧值并设置新值
            freeValue(*structValue->fields[i].value);
            *structValue->fields[i].value = copyValue(value);
            
            // 对于结构体字段赋值，我们需要更新变量环境中的结构体
            // 这需要找到原始变量并更新它
            if (expr->as.structAssign.object->type == EXPR_VARIABLE) {
                Token varName = expr->as.structAssign.object->as.variable.name;
                assignVariable(interpreter->environment, varName, objectValue);
            }
            
            freeValue(objectValue);
            return value;
        }
    }
    
    // 字段未找到
    freeValue(value);
    freeValue(objectValue);
    runtimeError(interpreter, "Struct field not found");
    return createNull();
}