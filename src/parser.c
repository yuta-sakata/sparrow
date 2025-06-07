#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

// 前向声明所有解析函数
static Expr *expression(Parser *parser);
static Expr *assignment(Parser *parser);
static Expr *equality(Parser *parser);
static Expr *comparison(Parser *parser);
static Expr *term(Parser *parser);
static Expr *factor(Parser *parser);
static Expr *unary(Parser *parser);
static Expr *call(Parser *parser);
static Expr *primary(Parser *parser);
static Expr *finishCall(Parser *parser, Expr *callee);

static Stmt *declaration(Parser *parser);
static Stmt *functionDeclaration(Parser *parser);
static Stmt *varDeclaration(Parser *parser);
static Stmt *statement(Parser *parser);
static Stmt *expressionStatement(Parser *parser);
static Stmt *blockStatement(Parser *parser);
static Stmt *ifStatement(Parser *parser);
static Stmt *whileStatement(Parser *parser);
static Stmt *forStatement(Parser *parser);
static Stmt *returnStatement(Parser *parser);

// 辅助函数声明
static int match(Parser *parser, TokenType type);
static int check(Parser *parser, TokenType type);
static Token advance(Parser *parser);
static Token peek(Parser *parser);
static Token previous(Parser *parser);
static int isAtEnd(Parser *parser);
static Token consume(Parser *parser, TokenType type, const char *message);
static void synchronize(Parser *parser);
static void error(Parser *parser, const char *message);

// 初始化语法分析器
void initParser(Parser *parser, Token *tokens, int count)
{
    parser->tokens = tokens;
    parser->count = count;
    parser->current = 0;
    parser->hadError = 0;
    parser->errorMsg[0] = '\0';
}

// 解析整个程序，返回语句列表
Stmt **parse(Parser *parser, int *stmtCount)
{
    int capacity = 8;
    Stmt **statements = (Stmt **)malloc(capacity * sizeof(Stmt *));
    int count = 0;

    while (!isAtEnd(parser))
    {
        // 保存当前位置以检测进度
        int currentPos = parser->current;

        parser->hadError = 0; // 为每个顶级声明重置错误标志
        Stmt *stmt = declaration(parser);

        if (stmt != NULL)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                statements = (Stmt **)realloc(statements, capacity * sizeof(Stmt *));
            }
            statements[count++] = stmt;
        }

        if (parser->hadError)
        {
            synchronize(parser);

            // 如果 synchronize 没有前进，手动前进以避免无限循环
            if (currentPos == parser->current && !isAtEnd(parser))
            {
                advance(parser);
            }
        }
    }

    *stmtCount = count;
    return statements;
}

// 检查是否有语法错误
int hadParseError(Parser *parser)
{
    return parser->hadError;
}

// 获取错误信息
const char *getParseErrorMsg(Parser *parser)
{
    return parser->errorMsg;
}

/**
 * @brief 解析声明语句
 *
 * 处理函数声明和变量声明。如果当前标记是函数关键字，则解析函数声明；
 * 如果当前标记是变量声明关键字(var)，则解析变量声明；
 * 否则将解析为普通语句。
 *
 * 对于变量声明，支持可选的类型注解（使用冒号后跟类型名），以及可选的初始化表达式。
 * 类型可以是int、float、string或bool。
 *
 * @param parser 解析器指针
 * @return Stmt* 返回解析后的声明语句，若解析出错则返回NULL
 */
static Stmt *declaration(Parser *parser)
{
    if (match(parser, TOKEN_FUNCTION))
    {
        return functionDeclaration(parser);
    }

    if (match(parser, TOKEN_VAR))
    {
        // 创建存储多个变量名的数组
        int capacity = 4;
        int count = 0;
        Token *names = (Token *)malloc(sizeof(Token) * capacity);

        // 解析第一个变量名
        Token name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
        names[count++] = name;

        // 处理连续的变量声明（用逗号分隔）
        while (match(parser, TOKEN_COMMA))
        {
            if (count >= capacity)
            {
                capacity *= 2;
                names = (Token *)realloc(names, sizeof(Token) * capacity);
            }

            name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name after ','.");
            if (parser->hadError)
            {
                free(names);
                return NULL;
            }
            names[count++] = name;
        }

        // 处理类型注解（所有变量共用相同类型）
        Token type = {0};
        type.type = TOKEN_VOID; // 默认值

        if (match(parser, TOKEN_COLON))
        {
            if (match(parser, TOKEN_INT))
            {
                type = previous(parser);
            }
            else if (match(parser, TOKEN_FLOAT_TYPE))
            {
                type = previous(parser);
            }
            else if (match(parser, TOKEN_STRING_TYPE))
            {
                type = previous(parser);
            }
            else if (match(parser, TOKEN_BOOL))
            {
                type = previous(parser);
            }
            else if (match(parser, TOKEN_VOID))
            {
                error(parser, "void can only be used as a function return type");
                free(names);
                return NULL;
            }
            else
            {
                error(parser, "Expected type annotation.");
                free(names);
                return NULL;
            }
        }

        // 处理初始值（可选，所有变量共用相同初始值）
        Expr *initializer = NULL;
        if (match(parser, TOKEN_ASSIGN))
        {
            initializer = expression(parser);
        }

        consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
        if (parser->hadError)
        {
            if (initializer)
                freeExpr(initializer);
            free(names);
            return NULL;
        }

        // 为多个变量创建声明语句
        if (count == 1)
        {
            // 只有一个变量，直接创建并返回单个语句
            Stmt *stmt = createVarStmt(names[0], type, initializer);
            free(names);
            return stmt;
        }
        else
        {
            return createMultiVarStmt(names, count, type, initializer);
        }
    }

    return statement(parser);
}

// 解析函数声明
static Stmt *functionDeclaration(Parser *parser)
{
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    if (parser->hadError)
        return NULL;

    consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    if (parser->hadError)
        return NULL;

    // 参数列表
    Token *parameters = NULL;
    Token *paramTypes = NULL;
    int paramCount = 0;
    int paramCapacity = 0;

    if (!check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (paramCount >= 255)
            {
                error(parser, "Cannot have more than 255 parameters.");
                return NULL;
            }

            // 参数名
            Token param = consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            if (parser->hadError)
                return NULL;

            // 参数类型
            Token paramType = {0};
            paramType.type = TOKEN_VOID; // 默认类型

            if (match(parser, TOKEN_COLON))
            {
                // 解析参数类型
                if (match(parser, TOKEN_INT))
                {
                    paramType = previous(parser);
                }
                else if (match(parser, TOKEN_FLOAT_TYPE))
                {
                    paramType = previous(parser);
                }
                else if (match(parser, TOKEN_STRING_TYPE))
                {
                    paramType = previous(parser);
                }
                else if (match(parser, TOKEN_BOOL))
                {
                    paramType = previous(parser);
                }
                else
                {
                    error(parser, "Expected parameter type after ':'.");
                    return NULL;
                }
            }

            // 添加参数
            if (paramCount >= paramCapacity)
            {
                paramCapacity = paramCapacity == 0 ? 8 : paramCapacity * 2;
                parameters = (Token *)realloc(parameters, paramCapacity * sizeof(Token));
                paramTypes = (Token *)realloc(paramTypes, paramCapacity * sizeof(Token));
            }

            parameters[paramCount] = param;
            paramTypes[paramCount] = paramType;
            paramCount++;

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    if (parser->hadError)
        return NULL;

    // 检查是否有返回类型注解（冒号后跟类型）
    Token returnType = {0};
    returnType.type = TOKEN_VOID; // 默认返回类型为 void

    if (match(parser, TOKEN_COLON))
    {
        // 解析返回类型
        if (match(parser, TOKEN_VOID))
        {
            returnType = previous(parser);
        }
        else if (match(parser, TOKEN_INT))
        {
            returnType = previous(parser);
        }
        else if (match(parser, TOKEN_FLOAT_TYPE))
        {
            returnType = previous(parser);
        }
        else if (match(parser, TOKEN_STRING_TYPE))
        {
            returnType = previous(parser);
        }
        else if (match(parser, TOKEN_BOOL))
        {
            returnType = previous(parser);
        }
        else
        {
            error(parser, "Expected return type after ':'.");
            return NULL;
        }
    }

    // 函数体
    consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    if (parser->hadError)
        return NULL;

    Stmt *body = blockStatement(parser);
    if (parser->hadError)
        return NULL;

    return createFunctionStmt(name, parameters, paramTypes, paramCount, returnType, body);
}

// 解析变量声明
static Stmt *varDeclaration(Parser *parser)
{
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    if (parser->hadError)
        return NULL;

    Token type;
    type.type = TOKEN_VOID; // 默认值，实际上会被忽略

    // 处理类型注解
    if (match(parser, TOKEN_COLON))
    {
        if (match(parser, TOKEN_INT))
        {
            type = previous(parser);
        }
        else if (match(parser, TOKEN_FLOAT_TYPE))
        {
            type = previous(parser);
        }
        else if (match(parser, TOKEN_STRING_TYPE))
        {
            type = previous(parser);
        }
        else if (match(parser, TOKEN_BOOL))
        {
            type = previous(parser);
        }
        else
        {
            error(parser, "Expected type annotation after ':'.");
            return NULL;
        }
    }

    Expr *initializer = NULL;
    if (match(parser, TOKEN_ASSIGN))
    {
        initializer = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    if (parser->hadError)
    {
        if (initializer)
            freeExpr(initializer);
        return NULL;
    }

    return createVarStmt(name, type, initializer);
}

// 解析语句
static Stmt *statement(Parser *parser)
{
    if (match(parser, TOKEN_IF))
    {
        return ifStatement(parser);
    }

    if (match(parser, TOKEN_WHILE))
    {
        return whileStatement(parser);
    }

    if (match(parser, TOKEN_FOR))
    {
        return forStatement(parser);
    }

    if (match(parser, TOKEN_RETURN))
    {
        return returnStatement(parser);
    }

    if (match(parser, TOKEN_LBRACE))
    {
        return blockStatement(parser);
    }

    return expressionStatement(parser);
}

// 解析表达式语句
static Stmt *expressionStatement(Parser *parser)
{

    Expr *expr = expression(parser);

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");

    if (parser->hadError)
    {
        freeExpr(expr);
        return NULL;
    }

    Stmt *stmt = createExpressionStmt(expr); // 先保存结果
    return stmt;                             // 添加这行以返回创建的语句
}

// 解析代码块
static Stmt *blockStatement(Parser *parser)
{
    int capacity = 8;
    Stmt **statements = (Stmt **)malloc(capacity * sizeof(Stmt *));
    int count = 0;

    while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        Stmt *stmt = declaration(parser);

        if (stmt != NULL)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                statements = (Stmt **)realloc(statements, capacity * sizeof(Stmt *));
            }
            statements[count++] = stmt;
        }
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    if (parser->hadError)
    {
        for (int i = 0; i < count; i++)
        {
            freeStmt(statements[i]);
        }
        free(statements);
        return NULL;
    }

    return createBlockStmt(statements, count);
}

// 解析if语句
static Stmt *ifStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    if (parser->hadError)
        return NULL;

    Expr *condition = expression(parser);

    consume(parser, TOKEN_RPAREN, "Expect ')' after if condition.");
    if (parser->hadError)
    {
        freeExpr(condition);
        return NULL;
    }

    Stmt *thenBranch = statement(parser);
    Stmt *elseBranch = NULL;

    if (match(parser, TOKEN_ELSE))
    {
        elseBranch = statement(parser);
    }

    return createIfStmt(condition, thenBranch, elseBranch);
}

// 解析while循环
static Stmt *whileStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    if (parser->hadError)
        return NULL;

    Expr *condition = expression(parser);

    consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    if (parser->hadError)
    {
        freeExpr(condition);
        return NULL;
    }

    Stmt *body = statement(parser);

    return createWhileStmt(condition, body);
}

// 解析for循环
static Stmt *forStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'for'.");
    if (parser->hadError)
        return NULL;

    // 初始化部分
    Stmt *initializer = NULL;
    if (match(parser, TOKEN_SEMICOLON))
    {
        // 没有初始化
    }
    else if (check(parser, TOKEN_VAR))
    {
        advance(parser);
        initializer = varDeclaration(parser);
    }
    else if (check(parser, TOKEN_INT) || check(parser, TOKEN_FLOAT_TYPE) ||
             check(parser, TOKEN_STRING_TYPE) || check(parser, TOKEN_BOOL))
    {
        initializer = varDeclaration(parser);
    }
    else
    {
        initializer = expressionStatement(parser);
    }

    if (parser->hadError)
        return NULL;

    // 条件部分
    Expr *condition = NULL;
    if (!check(parser, TOKEN_SEMICOLON))
    {
        condition = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    if (parser->hadError)
    {
        if (initializer)
            freeStmt(initializer);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    // 增量部分
    Expr *increment = NULL;
    if (!check(parser, TOKEN_RPAREN))
    {
        increment = expression(parser);
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after for clauses.");
    if (parser->hadError)
    {
        if (initializer)
            freeStmt(initializer);
        if (condition)
            freeExpr(condition);
        if (increment)
            freeExpr(increment);
        return NULL;
    }

    // 循环体
    Stmt *body = statement(parser);

    return createForStmt(initializer, condition, increment, body);
}

// 解析return语句
static Stmt *returnStatement(Parser *parser)
{
    Token keyword = previous(parser);
    Expr *value = NULL;

    if (!check(parser, TOKEN_SEMICOLON))
    {
        value = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    if (parser->hadError)
    {
        if (value)
            freeExpr(value);
        return NULL;
    }

    return createReturnStmt(keyword, value);
}

// 解析表达式
static Expr *expression(Parser *parser)
{
    return assignment(parser);
}

// 解析赋值表达式
static Expr *assignment(Parser *parser)
{
    Expr *expr = equality(parser);

    if (match(parser, TOKEN_ASSIGN))
    {
        Token equals = previous(parser);
        Expr *value = assignment(parser);

        if (expr->type == EXPR_VARIABLE)
        {
            Token name = expr->as.variable.name;
            freeExpr(expr);
            return createAssignExpr(name, value);
        }

        error(parser, "Invalid assignment target.");
        freeExpr(value);
    }

    return expr;
}

// 解析相等性表达式
static Expr *equality(Parser *parser)
{
    Expr *expr = comparison(parser);

    while (match(parser, TOKEN_EQ) || match(parser, TOKEN_NE))
    {
        TokenType operator = previous(parser).type;
        Expr *right = comparison(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析比较表达式
static Expr *comparison(Parser *parser)
{
    Expr *expr = term(parser);

    while (match(parser, TOKEN_LT) || match(parser, TOKEN_LE) ||
           match(parser, TOKEN_GT) || match(parser, TOKEN_GE))
    {
        TokenType operator = previous(parser).type;
        Expr *right = term(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析加减表达式
static Expr *term(Parser *parser)
{
    Expr *expr = factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = factor(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析乘除表达式
static Expr *factor(Parser *parser)
{
    Expr *expr = unary(parser);

    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE) || match(parser, TOKEN_MODULO))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析一元表达式
static Expr *unary(Parser *parser)
{
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        return createUnaryExpr(operator, right);
    }

    return call(parser);
}

// 解析调用表达式
static Expr *call(Parser *parser)
{
    Expr *expr = primary(parser);

    while (true)
    {
        if (match(parser, TOKEN_LPAREN))
        {
            expr = finishCall(parser, expr);
        }
        else if (match(parser, TOKEN_PLUS_PLUS) || match(parser, TOKEN_MINUS_MINUS))
        {
            TokenType op = previous(parser).type;

            // 检查左操作数是否是变量
            if (expr->type != EXPR_VARIABLE)
            {
                error(parser, "Invalid left-hand side in postfix expression.");
                freeExpr(expr);
                return NULL;
            }

            expr = createPostfixExpr(expr, op);
        }
        else
        {
            break;
        }
    }

    return expr;
}

// 完成函数调用的解析
static Expr *finishCall(Parser *parser, Expr *callee)
{
    // 解析参数列表
    Expr **arguments = NULL;
    int argCount = 0;
    int capacity = 0;

    if (!check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (argCount >= 255)
            {
                error(parser, "Cannot have more than 255 arguments.");
                break;
            }

            Expr *arg = expression(parser);

            if (argCount >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                arguments = (Expr **)realloc(arguments, capacity * sizeof(Expr *));
            }

            arguments[argCount++] = arg;

        } while (match(parser, TOKEN_COMMA));
    }

    Token paren = consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    if (parser->hadError)
    {
        for (int i = 0; i < argCount; i++)
        {
            freeExpr(arguments[i]);
        }
        free(arguments);
        freeExpr(callee);
        return NULL;
    }

    return createCallExpr(callee, paren, arguments, argCount);
}

// 解析基本表达式
static Expr *primary(Parser *parser)
{
    if (match(parser, TOKEN_FALSE))
    {
        Token token = previous(parser);
        token.type = TOKEN_FALSE;
        return createLiteralExpr(token);
    }

    if (match(parser, TOKEN_TRUE))
    {
        Token token = previous(parser);
        token.type = TOKEN_TRUE;
        return createLiteralExpr(token);
    }

    if (match(parser, TOKEN_NULL))
    {
        Token token = previous(parser);
        token.type = TOKEN_NULL;
        return createLiteralExpr(token);
    }

    if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT) || match(parser, TOKEN_STRING))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        return createVariableExpr(previous(parser));
    }

    if (match(parser, TOKEN_LPAREN))
    {
        Expr *expr = expression(parser);
        consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        if (parser->hadError)
        {
            freeExpr(expr);
            return NULL;
        }
        return createGroupingExpr(expr);
    }

    error(parser, "Expect expression.");
    return NULL;
}

// 检查当前标记是否匹配指定类型
static int match(Parser *parser, TokenType type)
{
    if (check(parser, type))
    {
        advance(parser);
        return 1;
    }
    return 0;
}

// 检查当前标记是否是指定类型
static int check(Parser *parser, TokenType type)
{
    if (isAtEnd(parser))
        return 0;
    return peek(parser).type == type;
}

// 前进到下一个标记并返回它
static Token advance(Parser *parser)
{
    if (!isAtEnd(parser))
        parser->current++;
    return previous(parser);
}

// 获取当前标记
static Token peek(Parser *parser)
{
    return parser->tokens[parser->current];
}

// 获取上一个标记
static Token previous(Parser *parser)
{
    return parser->tokens[parser->current - 1];
}

// 检查是否到达标记流末尾
static int isAtEnd(Parser *parser)
{
    return peek(parser).type == TOKEN_EOF;
}

// 消费当前标记，如果类型匹配则前进，否则报错
static Token consume(Parser *parser, TokenType type, const char *message)
{
    if (check(parser, type))
    {
        return advance(parser);
    }

    error(parser, message);
    Token errorToken = {0};
    return errorToken;
}

// 错误处理：同步到下一个安全点
static void synchronize(Parser *parser)
{
    parser->hadError = 0; // 重置错误标志，尝试继续解析
    advance(parser);

    while (!isAtEnd(parser))
    {
        if (previous(parser).type == TOKEN_SEMICOLON)
            return;

        switch (peek(parser).type)
        {
        case TOKEN_FUNCTION:
        case TOKEN_INT:
        case TOKEN_FLOAT_TYPE:
        case TOKEN_STRING_TYPE:
        case TOKEN_BOOL:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_FOR:
        case TOKEN_RETURN:
        case TOKEN_LBRACE:
        case TOKEN_RBRACE: // 添加右花括号作为同步点
            return;
        }

        advance(parser);
    }
}

// 记录错误
static void error(Parser *parser, const char *message)
{
    parser->hadError = 1;
    snprintf(parser->errorMsg, sizeof(parser->errorMsg), "Line %d: Error: %s",
             peek(parser).line, message);
}