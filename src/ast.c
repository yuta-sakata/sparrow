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
Stmt *createVarStmt(Token name, Token typeToken, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;

    // 将 Token 类型转换为 TypeAnnotation
    stmt->as.var.type = tokenToTypeAnnotation(typeToken.type);

    stmt->as.var.initializer = initializer;
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
Stmt *createFunctionStmt(Token name, Token *params, Token *paramTypes,
                         int paramCount, Token returnTypeToken, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->as.function.name = name;
    stmt->as.function.params = params;
    stmt->as.function.paramCount = paramCount;

    // 分配并转换参数类型
    TypeAnnotation *convertedParamTypes = NULL;
    if (paramCount > 0)
    {
        convertedParamTypes = (TypeAnnotation *)malloc(paramCount * sizeof(TypeAnnotation));
        for (int i = 0; i < paramCount; i++)
        {
            convertedParamTypes[i] = tokenToTypeAnnotation(paramTypes[i].type);
        }
    }
    stmt->as.function.paramTypes = convertedParamTypes;

    // 转换返回类型
    stmt->as.function.returnType = tokenToTypeAnnotation(returnTypeToken.type);

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
        freeStmt(stmt->as.function.body);
        free(stmt->as.function.params);
        free(stmt->as.function.paramTypes);
        break;
    case STMT_RETURN:
        if (stmt->as.returnStmt.value)
        {
            freeExpr(stmt->as.returnStmt.value);
        }
        break;
    }

    free(stmt);
}