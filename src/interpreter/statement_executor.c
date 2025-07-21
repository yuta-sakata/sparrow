#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

// 前向声明内部函数
static void executeExpression(Interpreter *interpreter, Stmt *stmt);
static void executeVar(Interpreter *interpreter, Stmt *stmt);
static void executeConst(Interpreter *interpreter, Stmt *stmt);
static void executeMultiVar(Interpreter *interpreter, Stmt *stmt);
static void executeMultiConst(Interpreter *interpreter, Stmt *stmt);
static void executeBlock(Interpreter *interpreter, Stmt **statements, int count, Environment *environment);
static void executeIf(Interpreter *interpreter, Stmt *stmt);
static void executeWhile(Interpreter *interpreter, Stmt *stmt);
static void executeDoWhile(Interpreter *interpreter, Stmt *stmt);
static void executeFor(Interpreter *interpreter, Stmt *stmt);
static void executeFunction(Interpreter *interpreter, Stmt *stmt);
static void executeReturn(Interpreter *interpreter, Stmt *stmt);
static void executeSwitch(Interpreter *interpreter, Stmt *stmt);
static void executeBreak(Interpreter *interpreter, Stmt *stmt);
static void executeEnum(Interpreter *interpreter, Stmt *stmt);
static void executeStruct(Interpreter *interpreter, Stmt *stmt);

void execute(Interpreter *interpreter, Stmt *stmt) {
    if (stmt == NULL)
        return;

    switch (stmt->type) {
    case STMT_EXPRESSION:
        executeExpression(interpreter, stmt);
        break;
    case STMT_VAR:
        executeVar(interpreter, stmt);
        break;
    case STMT_CONST:
        executeConst(interpreter, stmt);
        break;
    case STMT_MULTI_VAR:
        executeMultiVar(interpreter, stmt);
        break;
    case STMT_MULTI_CONST:
        executeMultiConst(interpreter, stmt);
        break;
    case STMT_BLOCK:
        executeBlock(interpreter, stmt->as.block.statements,
                     stmt->as.block.count,
                     interpreter->environment);
        break;
    case STMT_IF:
        executeIf(interpreter, stmt);
        break;
    case STMT_WHILE:
        executeWhile(interpreter, stmt);
        break;
    case STMT_DO_WHILE:
        executeDoWhile(interpreter, stmt);
        break;
    case STMT_FOR:
        executeFor(interpreter, stmt);
        break;
    case STMT_FUNCTION:
        executeFunction(interpreter, stmt);
        break;
    case STMT_RETURN:
        executeReturn(interpreter, stmt);
        break;
    case STMT_SWITCH:
        executeSwitch(interpreter, stmt);
        break;
    case STMT_BREAK:
        executeBreak(interpreter, stmt);
        break;
    case STMT_ENUM:
        executeEnum(interpreter, stmt);
        break;
    case STMT_STRUCT:
        executeStruct(interpreter, stmt);
        break;
    default:
        break;
    }
}

static void executeExpression(Interpreter *interpreter, Stmt *stmt) {
    Value value = evaluate(interpreter, stmt->as.expression.expression);
    freeValue(value);
}

static void executeVar(Interpreter *interpreter, Stmt *stmt) {
    Value value = createNull();

    if (stmt->as.var.initializer != NULL) {
        value = evaluate(interpreter, stmt->as.var.initializer);
    }

    if (stmt->as.var.isStatic) {
        defineStaticVariable(interpreter->staticStorage, stmt->as.var.name.lexeme, value, false);
    } else {
        defineVariable(interpreter->environment, stmt->as.var.name.lexeme, value);
    }

    freeValue(value);
}

static void executeConst(Interpreter *interpreter, Stmt *stmt)
{
    Value value = createNull();

    if (stmt->as.constStmt.initializer != NULL)
    {
        value = evaluate(interpreter, stmt->as.constStmt.initializer);
        if (interpreter->hadError)
        {
            freeValue(value);
            return;
        }
    }
    else
    {
        runtimeError(interpreter, "Constants must be initialized.");
        return;
    }

    if (stmt->as.constStmt.isStatic)
    {
        // 静态常量存储在静态存储中
        defineStaticVariable(interpreter->staticStorage, stmt->as.constStmt.name.lexeme, value, true);
    }
    else
    {
        // 普通常量存储在当前环境中
        defineConstant(interpreter->environment, stmt->as.constStmt.name.lexeme, value);
    }
}

static void executeMultiVar(Interpreter *interpreter, Stmt *stmt) {
    Value initialValue = createNull();
    if (stmt->as.multiVar.initializer != NULL) {
        initialValue = evaluate(interpreter, stmt->as.multiVar.initializer);
        if (interpreter->hadError) {
            freeValue(initialValue);
            return;
        }
    }

    for (int i = 0; i < stmt->as.multiVar.count; i++) {
        if (stmt->as.multiVar.isStatic) {
            defineStaticVariable(interpreter->staticStorage, stmt->as.multiVar.names[i].lexeme, initialValue, false);
        } else {
            Value valueCopy = copyValue(initialValue);
            defineVariable(interpreter->environment, stmt->as.multiVar.names[i].lexeme, valueCopy);
            freeValue(valueCopy);
        }
    }

    freeValue(initialValue);
}

static void executeMultiConst(Interpreter *interpreter, Stmt *stmt) {
    if (stmt->as.multiConst.initializers == NULL || stmt->as.multiConst.initializerCount == 0) {
        runtimeError(interpreter, "Constants must be initialized.");
        return;
    }

    for (int i = 0; i < stmt->as.multiConst.count; i++) {
        Value value;
        
        if (stmt->as.multiConst.initializerCount == 1) {
            // 所有常量共享一个初始值
            value = evaluate(interpreter, stmt->as.multiConst.initializers[0]);
        } else if (i < stmt->as.multiConst.initializerCount) {
            // 每个常量有自己的初始值
            value = evaluate(interpreter, stmt->as.multiConst.initializers[i]);
        } else {
            runtimeError(interpreter, "Not enough initializers for constants.");
            return;
        }

        if (interpreter->hadError) {
            freeValue(value);
            return;
        }

        if (stmt->as.multiConst.isStatic) {
            defineStaticVariable(interpreter->staticStorage, stmt->as.multiConst.names[i].lexeme, value, true);
        } else {
            defineConstant(interpreter->environment, stmt->as.multiConst.names[i].lexeme, value);
        }

        freeValue(value);
    }
}

static void executeBlock(Interpreter *interpreter, Stmt **statements, int count, Environment *environment) {
    Environment *previous = interpreter->environment;
    Environment blockEnv;
    initEnvironment(&blockEnv, interpreter->environment);
    interpreter->environment = &blockEnv;

    for (int i = 0; i < count; i++) {
        execute(interpreter, statements[i]);
        if (interpreter->hadError)
            break;
        if (breakStatus.hasBreak)
            break;
        if (returnStatus.hasReturn)
            break;
    }

    interpreter->environment = previous;
    freeEnvironment(&blockEnv);
}

static void executeIf(Interpreter *interpreter, Stmt *stmt) {
    Value condition = evaluate(interpreter, stmt->as.ifStmt.condition);
    bool isTruthy = condition.type != VAL_NULL &&
                    !(condition.type == VAL_BOOL && !condition.as.boolean);
    freeValue(condition);

    if (isTruthy) {
        execute(interpreter, stmt->as.ifStmt.thenBranch);
    } else if (stmt->as.ifStmt.elseBranch != NULL) {
        execute(interpreter, stmt->as.ifStmt.elseBranch);
    }
}

static void executeWhile(Interpreter *interpreter, Stmt *stmt) {
    while (true) {
        Value condition = evaluate(interpreter, stmt->as.whileLoop.condition);
        bool isTruthy = condition.type != VAL_NULL &&
                        !(condition.type == VAL_BOOL && !condition.as.boolean);
        freeValue(condition);

        if (!isTruthy)
            break;

        execute(interpreter, stmt->as.whileLoop.body);
        if (interpreter->hadError)
            break;
        if (breakStatus.hasBreak) {
            breakStatus.hasBreak = false;
            break;
        }
        if (returnStatus.hasReturn)
            break;
    }
}

static void executeDoWhile(Interpreter *interpreter, Stmt *stmt) {
    do {
        execute(interpreter, stmt->as.doWhile.body);
        if (interpreter->hadError)
            break;
        if (breakStatus.hasBreak) {
            breakStatus.hasBreak = false;
            break;
        }
        if (returnStatus.hasReturn)
            break;

        Value condition = evaluate(interpreter, stmt->as.doWhile.condition);
        bool isTruthy = condition.type != VAL_NULL &&
                        !(condition.type == VAL_BOOL && !condition.as.boolean);
        freeValue(condition);

        if (!isTruthy)
            break;
    } while (true);
}

static void executeFor(Interpreter *interpreter, Stmt *stmt) {
    if (stmt->as.forLoop.initializer != NULL) {
        execute(interpreter, stmt->as.forLoop.initializer);
    }

    while (true) {
        if (stmt->as.forLoop.condition != NULL) {
            Value condition = evaluate(interpreter, stmt->as.forLoop.condition);
            bool isTruthy = condition.type != VAL_NULL &&
                            !(condition.type == VAL_BOOL && !condition.as.boolean);
            freeValue(condition);

            if (!isTruthy)
                break;
        }

        execute(interpreter, stmt->as.forLoop.body);
        if (interpreter->hadError)
            break;
        if (breakStatus.hasBreak) {
            breakStatus.hasBreak = false;
            break;
        }
        if (returnStatus.hasReturn)
            break;

        if (stmt->as.forLoop.increment != NULL) {
            Value incrementResult = evaluate(interpreter, stmt->as.forLoop.increment);
            freeValue(incrementResult);
        }
    }
}

static void executeFunction(Interpreter *interpreter, Stmt *stmt)
{
    // 创建函数对象
    Function *function = (Function *)malloc(sizeof(Function));
    if (function == NULL)
    {
        runtimeError(interpreter, "内存分配失败");
        return;
    }

    // 设置函数名
    size_t nameLen = strlen(stmt->as.function.name.lexeme);
    function->name = (char *)malloc(nameLen + 1);
    if (function->name == NULL)
    {
        free(function);
        runtimeError(interpreter, "内存分配失败");
        return;
    }
    strcpy(function->name, stmt->as.function.name.lexeme);

    function->arity = stmt->as.function.paramCount;
    function->paramTypes = NULL;
    function->returnType = stmt->as.function.returnType;

    // 分配参数名数组
    if (function->arity > 0)
    {
        function->paramNames = (char **)malloc(sizeof(char *) * function->arity);
        if (function->paramNames == NULL)
        {
            free(function->name);
            free(function);
            runtimeError(interpreter, "内存分配失败");
            return;
        }

        // 复制参数名
        for (int i = 0; i < function->arity; i++)
        {
            size_t paramNameLen = strlen(stmt->as.function.params[i].lexeme);
            function->paramNames[i] = (char *)malloc(paramNameLen + 1);
            if (function->paramNames[i] == NULL)
            {
                // 清理已分配的内存
                for (int j = 0; j < i; j++)
                {
                    free(function->paramNames[j]);
                }
                free(function->paramNames);
                free(function->name);
                free(function);
                runtimeError(interpreter, "内存分配失败");
                return;
            }
            strcpy(function->paramNames[i], stmt->as.function.params[i].lexeme);
        }
    }
    else
    {
        function->paramNames = NULL;
    }

    // 设置函数体
    function->body = stmt->as.function.body;

    // 设置闭包环境（当前为全局环境）
    function->closure = interpreter->globals;

    // 检查是否是 main 函数
    if (strcmp(function->name, "main") == 0)
    {
        interpreter->hasMainFunction = true;
        interpreter->mainFunction = function;
    }

    // 创建函数值并定义到环境中
    Value functionValue;
    functionValue.type = VAL_FUNCTION;
    functionValue.as.function = function;

    // 检查是否是静态函数
    if (stmt->as.function.isStatic)
    {
        // 静态函数在静态存储中定义
        defineStaticVariable(interpreter->staticStorage, function->name, functionValue, true);
    }
    else
    {
        // 普通函数在全局环境中定义
        defineVariable(interpreter->globals, function->name, functionValue);
    }
}

static void executeReturn(Interpreter *interpreter, Stmt *stmt) {
    Value value = createNull();

    if (stmt->as.returnStmt.value != NULL) {
        value = evaluate(interpreter, stmt->as.returnStmt.value);
    }

    returnStatus.hasReturn = true;
    returnStatus.value = value;
}

static void executeSwitch(Interpreter *interpreter, Stmt *stmt) {
    Value discriminant = evaluate(interpreter, stmt->as.switchStmt.discriminant);
    if (interpreter->hadError) {
        return;
    }

    bool matched = false;
    bool fallthrough = false;
    breakStatus.hasBreak = false;

    for (int i = 0; i < stmt->as.switchStmt.caseCount; i++) {
        CaseStmt *caseStmt = &stmt->as.switchStmt.cases[i];

        if (caseStmt->value == NULL) { // default case
            if (!matched) {
                matched = true;
                fallthrough = true;
            }
        } else {
            Value caseValue = evaluate(interpreter, caseStmt->value);
            if (interpreter->hadError) {
                freeValue(discriminant);
                return;
            }

            if (!matched && valuesEqual(discriminant, caseValue)) {
                matched = true;
                fallthrough = true;
            }

            freeValue(caseValue);
        }

        if (fallthrough) {
            execute(interpreter, caseStmt->body);
            if (breakStatus.hasBreak || interpreter->hadError) {
                break;
            }
        }
    }

    freeValue(discriminant);
    breakStatus.hasBreak = false;
}

static void executeBreak(Interpreter *interpreter, Stmt *stmt) {
    breakStatus.hasBreak = true;
}

static void executeEnum(Interpreter *interpreter, Stmt *stmt) {
    const char *enumName = stmt->as.enumStmt.name.lexeme;
    int currentValue = 0;

    for (int i = 0; i < stmt->as.enumStmt.memberCount; i++) {
        EnumMember *member = &stmt->as.enumStmt.members[i];

        int enumValue = 0;
        if (member->value != NULL) {
            Value val = evaluate(interpreter, member->value);
            if (interpreter->hadError)
                return;

            if (val.type == VAL_NUMBER) {
                enumValue = (int)val.as.number;
                currentValue = enumValue;
            } else {
                runtimeError(interpreter, "Enum value must be a number");
                freeValue(val);
                return;
            }
            freeValue(val);
        } else {
            enumValue = currentValue;
        }

        Value enumVal = createNumber((double)enumValue);

        char *fullName = malloc(strlen(enumName) + strlen(member->name.lexeme) + 2);
        if (fullName == NULL) {
            runtimeError(interpreter, "Memory allocation failed");
            freeValue(enumVal);
            return;
        }

        sprintf(fullName, "%s_%s", enumName, member->name.lexeme);
        defineConstant(interpreter->globals, fullName, enumVal);

        free(fullName);
        freeValue(enumVal);
        currentValue++;
    }
}

static void executeStruct(Interpreter *interpreter, Stmt *stmt) {
    // 结构体声明只需要存储结构体的元数据
    // 在这个简单实现中，我们将结构体信息存储在全局环境中
    // 实际使用时，结构体字面量会创建具体的结构体实例
    
    const char *structName = stmt->as.structStmt.name.lexeme;
    
    // 将结构体类型标记存储在全局环境中，这样解析器知道这是一个有效的结构体类型
    Value structTypeMarker = createString(structName);
    
    // 使用特殊前缀来标识这是一个结构体类型定义
    char *typeName = malloc(strlen(structName) + 8); // "struct_" + name + '\0'
    if (typeName == NULL) {
        runtimeError(interpreter, "Memory allocation failed");
        freeValue(structTypeMarker);
        return;
    }
    
    sprintf(typeName, "struct_%s", structName);
    defineConstant(interpreter->globals, typeName, structTypeMarker);
    
    free(typeName);
    freeValue(structTypeMarker);
}