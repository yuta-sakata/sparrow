#include "ast.h"

// 创建二元表达式
Expr *createBinaryExpr(Expr *left, TokenType op, Expr *right)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.op = op;
    expr->as.binary.right = right;
    return expr;
}

// 创建一元表达式
Expr *createUnaryExpr(TokenType op, Expr *right)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.op = op;
    expr->as.unary.right = right;
    return expr;
}

// 创建字面量表达式
Expr *createLiteralExpr(Token value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    return expr;
}

// 创建分组表达式
Expr *createGroupingExpr(Expr *expression)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;
    return expr;
}

// 创建变量引用
Expr *createVariableExpr(Token name)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;
    return expr;
}

// 创建赋值表达式
Expr *createAssignExpr(Token name, Expr *value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;
    return expr;
}

// 创建函数调用
Expr *createCallExpr(Expr *callee, Token paren, Expr **arguments, int argCount)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->as.call.callee = callee;
    expr->as.call.paren = paren;
    expr->as.call.arguments = arguments;
    expr->as.call.argCount = argCount;
    return expr;
}

Expr *createPostfixExpr(Expr *operand, TokenType op)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_POSTFIX;
    expr->as.postfix.operand = operand;
    expr->as.postfix.op = op;
    return expr;
}

Expr *createPrefixExpr(Expr *operand, TokenType op)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
    {
        return NULL;
    }
    expr->type = EXPR_PREFIX;
    expr->as.prefix.operand = operand;
    expr->as.prefix.op = op;
    return expr;
}

// 创建多变量声明
Stmt *createMultiVarStmt(Token *names, int count, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_MULTI_VAR;

    stmt->as.multiVar.names = names; // 直接使用传入的名称数组
    stmt->as.multiVar.count = count;
    stmt->as.multiVar.type = type;
    stmt->as.multiVar.initializer = initializer;

    return stmt;
}

// 创建表达式语句
Stmt *createExpressionStmt(Expr *expression)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        // 处理内存分配失败
        return NULL;
    }
    stmt->type = STMT_EXPRESSION;
    stmt->as.expression.expression = expression;
    return stmt;
}

// 创建变量声明
Stmt *createVarStmt(Token name, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;
    stmt->as.var.type = type;

    stmt->as.var.initializer = initializer;
    return stmt;
}

// 创建常量声明
Stmt *createConstStmt(Token name, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_CONST;
    stmt->as.constStmt.name = name;
    stmt->as.constStmt.type = type;
    stmt->as.constStmt.initializer = initializer;
    return stmt;
}

// 创建代码块
Stmt *createBlockStmt(Stmt **statements, int count)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.count = count;
    return stmt;
}

// 创建if语句
Stmt *createIfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->as.ifStmt.condition = condition;
    stmt->as.ifStmt.thenBranch = thenBranch;
    stmt->as.ifStmt.elseBranch = elseBranch;
    return stmt;
}

// 创建while循环
Stmt *createWhileStmt(Expr *condition, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->as.whileLoop.condition = condition;
    stmt->as.whileLoop.body = body;
    return stmt;
}

// 创建for循环
Stmt *createForStmt(Stmt *initializer, Expr *condition, Expr *increment, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_FOR;
    stmt->as.forLoop.initializer = initializer;
    stmt->as.forLoop.condition = condition;
    stmt->as.forLoop.increment = increment;
    stmt->as.forLoop.body = body;
    return stmt;
}

// 创建函数声明
Stmt *createFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnType, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->as.function.name = name;
    stmt->as.function.params = params;
    stmt->as.function.paramHasVar = paramHasVar;
    stmt->as.function.paramCount = paramCount;

    // 分配并复制参数类型
    TypeAnnotation *convertedParamTypes = NULL;
    if (paramCount > 0)
    {
        convertedParamTypes = (TypeAnnotation *)malloc(paramCount * sizeof(TypeAnnotation));
        for (int i = 0; i < paramCount; i++)
        {
            convertedParamTypes[i] = paramTypes[i];
        }
    }
    stmt->as.function.paramTypes = convertedParamTypes;

    // 直接使用返回类型，不需要转换
    stmt->as.function.returnType = returnType;

    stmt->as.function.body = body;
    return stmt;
}
// 创建return语句
Stmt *createReturnStmt(Token keyword, Expr *value)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.returnStmt.keyword = keyword;
    stmt->as.returnStmt.value = value;
    return stmt;
}

// 创建 switch 语句
Stmt *createSwitchStmt(Expr *discriminant, CaseStmt *cases, int caseCount)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_SWITCH;
    stmt->as.switchStmt.discriminant = discriminant;
    stmt->as.switchStmt.cases = cases;
    stmt->as.switchStmt.caseCount = caseCount;
    return stmt;
}

// 创建 break 语句
Stmt *createBreakStmt(Token keyword)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_BREAK;
    stmt->as.breakStmt.keyword = keyword;
    return stmt;
}

Expr *copyExpr(Expr *expr)
{
    if (expr == NULL)
        return NULL;

    switch (expr->type)
    {
    case EXPR_BINARY:
    {
        Expr *leftCopy = copyExpr(expr->as.binary.left);
        Expr *rightCopy = copyExpr(expr->as.binary.right);
        return createBinaryExpr(leftCopy, expr->as.binary.op, rightCopy);
    }

    case EXPR_UNARY:
    {
        Expr *rightCopy = copyExpr(expr->as.unary.right);
        return createUnaryExpr(expr->as.unary.op, rightCopy);
    }

    case EXPR_LITERAL:
    {
        // 复制Token
        Token tokenCopy = expr->as.literal.value;

        // 如果是字符串类型，需要手动深度复制字符串内容
        if (tokenCopy.type == TOKEN_STRING && tokenCopy.value.stringValue != NULL)
        {
            size_t strLen = strlen(tokenCopy.value.stringValue);
            char *strCopy = (char *)malloc(strLen + 1); // +1 for null terminator
            if (strCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(strCopy, tokenCopy.value.stringValue, strLen);
            strCopy[strLen] = '\0'; // 确保字符串以null结尾
            tokenCopy.value.stringValue = strCopy;
        }

        // 如果token.lexeme需要复制
        if (tokenCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(tokenCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                if (tokenCopy.type == TOKEN_STRING)
                    free(tokenCopy.value.stringValue);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, tokenCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            tokenCopy.lexeme = lexCopy;
        }

        return createLiteralExpr(tokenCopy);
    }

    case EXPR_GROUPING:
    {
        Expr *exprCopy = copyExpr(expr->as.grouping.expression);
        return createGroupingExpr(exprCopy);
    }

    case EXPR_VARIABLE:
    {
        // 复制Token
        Token nameCopy = expr->as.variable.name;
        if (nameCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(nameCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, nameCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            nameCopy.lexeme = lexCopy;
        }
        return createVariableExpr(nameCopy);
    }

    case EXPR_ASSIGN:
    {
        // 复制Token
        Token nameCopy = expr->as.assign.name;
        if (nameCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(nameCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, nameCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            nameCopy.lexeme = lexCopy;
        }

        Expr *valueCopy = copyExpr(expr->as.assign.value);
        return createAssignExpr(nameCopy, valueCopy);
    }

    case EXPR_CALL:
    {
        Expr *calleeCopy = copyExpr(expr->as.call.callee);

        // 复制Token
        Token parenCopy = expr->as.call.paren;
        if (parenCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(parenCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                freeExpr(calleeCopy);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, parenCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            parenCopy.lexeme = lexCopy;
        }

        // 复制参数
        Expr **argsCopy = NULL;
        if (expr->as.call.argCount > 0)
        {
            argsCopy = (Expr **)malloc(sizeof(Expr *) * expr->as.call.argCount);
            if (argsCopy == NULL)
            {
                freeExpr(calleeCopy);
                if (parenCopy.lexeme != NULL)
                    free(parenCopy.lexeme);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }

            // 初始化为NULL，以便在出错时正确清理
            for (int i = 0; i < expr->as.call.argCount; i++)
                argsCopy[i] = NULL;

            // 复制每个参数
            for (int i = 0; i < expr->as.call.argCount; i++)
            {
                argsCopy[i] = copyExpr(expr->as.call.arguments[i]);
                if (argsCopy[i] == NULL)
                {
                    // 清理已复制的参数
                    for (int j = 0; j < i; j++)
                        freeExpr(argsCopy[j]);
                    free(argsCopy);
                    freeExpr(calleeCopy);
                    if (parenCopy.lexeme != NULL)
                        free(parenCopy.lexeme);
                    return NULL;
                }
            }
        }

        return createCallExpr(calleeCopy, parenCopy, argsCopy, expr->as.call.argCount);
    }

    case EXPR_POSTFIX:
    {
        Expr *operandCopy = copyExpr(expr->as.postfix.operand);
        return createPostfixExpr(operandCopy, expr->as.postfix.op);
    }

    case EXPR_PREFIX:
    {
        Expr *operandCopy = copyExpr(expr->as.prefix.operand);
        return createPrefixExpr(operandCopy, expr->as.prefix.op);
    }

    case EXPR_ARRAY_LITERAL:
    {
        Expr **elementsCopy = NULL;
        if (expr->as.arrayLiteral.elementCount > 0)
        {
            elementsCopy = (Expr **)malloc(sizeof(Expr *) * expr->as.arrayLiteral.elementCount);
            if (elementsCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }

            for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++)
            {
                elementsCopy[i] = copyExpr(expr->as.arrayLiteral.elements[i]);
                if (elementsCopy[i] == NULL)
                {
                    // 清理已复制的元素
                    for (int j = 0; j < i; j++)
                        freeExpr(elementsCopy[j]);
                    free(elementsCopy);
                    return NULL;
                }
            }
        }
        return createArrayLiteralExpr(elementsCopy, expr->as.arrayLiteral.elementCount);
    }

    case EXPR_ARRAY_ACCESS:
    {
        Expr *arrayCopy = copyExpr(expr->as.arrayAccess.array);
        Expr *indexCopy = copyExpr(expr->as.arrayAccess.index);
        if (arrayCopy == NULL || indexCopy == NULL)
        {
            if (arrayCopy)
                freeExpr(arrayCopy);
            if (indexCopy)
                freeExpr(indexCopy);
            return NULL;
        }
        return createArrayAccessExpr(arrayCopy, indexCopy);
    }

    case EXPR_ARRAY_ASSIGN:
    {
        Expr *arrayCopy = copyExpr(expr->as.arrayAssign.array);
        Expr *indexCopy = copyExpr(expr->as.arrayAssign.index);
        Expr *valueCopy = copyExpr(expr->as.arrayAssign.value);
        if (arrayCopy == NULL || indexCopy == NULL || valueCopy == NULL)
        {
            if (arrayCopy)
                freeExpr(arrayCopy);
            if (indexCopy)
                freeExpr(indexCopy);
            if (valueCopy)
                freeExpr(valueCopy);
            return NULL;
        }
        return createArrayAssignExpr(arrayCopy, indexCopy, valueCopy);
    }

    case EXPR_CAST:
    {
        Expr *exprCopy = copyExpr(expr->as.cast.expression);
        if (exprCopy == NULL)
        {
            return NULL;
        }
        return createCastExpr(expr->as.cast.targetType, exprCopy);
    }

    default:
        fprintf(stderr, "未知的表达式类型\n");
        return NULL;
    }
}

// 创建数组字面量表达式
Expr *createArrayLiteralExpr(Expr **elements, int elementCount)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_ARRAY_LITERAL;
    expr->as.arrayLiteral.elements = elements;
    expr->as.arrayLiteral.elementCount = elementCount;
    return expr;
}

// 创建数组访问表达式
Expr *createArrayAccessExpr(Expr *array, Expr *index)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_ARRAY_ACCESS;
    expr->as.arrayAccess.array = array;
    expr->as.arrayAccess.index = index;
    return expr;
}

// 创建数组赋值表达式
Expr *createArrayAssignExpr(Expr *array, Expr *index, Expr *value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_ARRAY_ASSIGN;
    expr->as.arrayAssign.array = array;
    expr->as.arrayAssign.index = index;
    expr->as.arrayAssign.value = value;
    return expr;
}

// 创建类型转换表达式
Expr *createCastExpr(BaseType targetType, Expr *expression)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_CAST;
    expr->as.cast.targetType = targetType;
    expr->as.cast.expression = expression;
    return expr;
}

// 释放表达式节点内存
void freeExpr(Expr *expr)
{
    if (expr == NULL)
        return;

    switch (expr->type)
    {
    case EXPR_BINARY:
        freeExpr(expr->as.binary.left);
        freeExpr(expr->as.binary.right);
        break;
    case EXPR_UNARY:
        freeExpr(expr->as.unary.right);
        break;
    case EXPR_GROUPING:
        freeExpr(expr->as.grouping.expression);
        break;
    case EXPR_ASSIGN:
        freeExpr(expr->as.assign.value);
        break;
    case EXPR_CALL:
        freeExpr(expr->as.call.callee);
        for (int i = 0; i < expr->as.call.argCount; i++)
        {
            freeExpr(expr->as.call.arguments[i]);
        }
        free(expr->as.call.arguments);
        break;
    case EXPR_POSTFIX:
        freeExpr(expr->as.postfix.operand);
        break;
    case EXPR_PREFIX:
        freeExpr(expr->as.prefix.operand);
        break;
    case EXPR_ARRAY_LITERAL:
        for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++)
        {
            freeExpr(expr->as.arrayLiteral.elements[i]);
        }
        free(expr->as.arrayLiteral.elements);
        break;

    case EXPR_ARRAY_ACCESS:
        freeExpr(expr->as.arrayAccess.array);
        freeExpr(expr->as.arrayAccess.index);
        break;

    case EXPR_ARRAY_ASSIGN:
        freeExpr(expr->as.arrayAssign.array);
        freeExpr(expr->as.arrayAssign.index);
        freeExpr(expr->as.arrayAssign.value);
        break;
    case EXPR_CAST:
        if (expr->as.cast.expression != NULL)
        {
            freeExpr(expr->as.cast.expression);
        }
        break;
    case EXPR_LITERAL:
    case EXPR_VARIABLE:
        // 这些节点没有需要释放的指针
        break;
    }

    free(expr);
}

// 释放语句节点内存
void freeStmt(Stmt *stmt)
{
    if (stmt == NULL)
        return;

    switch (stmt->type)
    {
    case STMT_EXPRESSION:
        freeExpr(stmt->as.expression.expression);
        break;
    case STMT_VAR:
        freeExpr(stmt->as.var.initializer);
        break;
    case STMT_CONST: // 添加常量声明的内存释放
        if (stmt->as.constStmt.initializer != NULL)
        {
            freeExpr(stmt->as.constStmt.initializer);
        }
        break;
    case STMT_MULTI_VAR:
        if (stmt->as.multiVar.initializer != NULL)
        {
            freeExpr(stmt->as.multiVar.initializer);
        }
        if (stmt->as.multiVar.names != NULL)
        {
            free(stmt->as.multiVar.names);
        }
        break;
    case STMT_BLOCK:
        for (int i = 0; i < stmt->as.block.count; i++)
        {
            freeStmt(stmt->as.block.statements[i]);
        }
        free(stmt->as.block.statements);
        break;
    case STMT_IF:
        freeExpr(stmt->as.ifStmt.condition);
        freeStmt(stmt->as.ifStmt.thenBranch);
        if (stmt->as.ifStmt.elseBranch)
        {
            freeStmt(stmt->as.ifStmt.elseBranch);
        }
        break;
    case STMT_WHILE:
        freeExpr(stmt->as.whileLoop.condition);
        freeStmt(stmt->as.whileLoop.body);
        break;
    case STMT_FOR:
        if (stmt->as.forLoop.initializer)
        {
            freeStmt(stmt->as.forLoop.initializer);
        }
        if (stmt->as.forLoop.condition)
        {
            freeExpr(stmt->as.forLoop.condition);
        }
        if (stmt->as.forLoop.increment)
        {
            freeExpr(stmt->as.forLoop.increment);
        }
        freeStmt(stmt->as.forLoop.body);
        break;
    case STMT_FUNCTION:
        if (stmt->as.function.params)
        {
            free(stmt->as.function.params);
        }
        if (stmt->as.function.paramHasVar)
        {
            free(stmt->as.function.paramHasVar);
        }
        if (stmt->as.function.paramTypes)
        {
            free(stmt->as.function.paramTypes);
        }
        if (stmt->as.function.body)
        {
            freeStmt(stmt->as.function.body);
        }
        break;
    case STMT_RETURN:
        if (stmt->as.returnStmt.value)
        {
            freeExpr(stmt->as.returnStmt.value);
        }
        break;
    case STMT_SWITCH:
        if (stmt->as.switchStmt.discriminant != NULL)
        {
            freeExpr(stmt->as.switchStmt.discriminant);
        }
        if (stmt->as.switchStmt.cases != NULL)
        {
            for (int i = 0; i < stmt->as.switchStmt.caseCount; i++)
            {
                if (stmt->as.switchStmt.cases[i].value != NULL)
                {
                    freeExpr(stmt->as.switchStmt.cases[i].value);
                }
                if (stmt->as.switchStmt.cases[i].body != NULL)
                {
                    freeStmt(stmt->as.switchStmt.cases[i].body);
                }
            }
            free(stmt->as.switchStmt.cases);
        }
        break;
    case STMT_BREAK:
        // break 语句没有需要释放的内存
        break;
    }

    free(stmt);
}