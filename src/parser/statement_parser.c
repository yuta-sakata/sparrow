#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/parser/statement_parser.h"
#include "../include/parser/declaration_parser.h"
#include "../include/parser/expression_parser.h"

// 解析语句
Stmt *statement(Parser *parser)
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

    if (match(parser, TOKEN_DO))
        return doWhileStatement(parser);

    if (match(parser, TOKEN_RETURN))
    {
        return returnStatement(parser);
    }

    if (match(parser, TOKEN_SWITCH))
    {
        return switchStatement(parser);
    }

    if (match(parser, TOKEN_BREAK))
    {
        return breakStatement(parser);
    }

    if (match(parser, TOKEN_LBRACE))
    {
        return blockStatement(parser);
    }

    return expressionStatement(parser);
}

// 解析表达式语句
Stmt *expressionStatement(Parser *parser)
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
Stmt *blockStatement(Parser *parser)
{
    int capacity = 8;
    Stmt **statements = (Stmt **)malloc(capacity * sizeof(Stmt *));
    int count = 0;

    while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        // 保存当前位置以检测进度
        int currentPos = parser->current;

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

        // 检查是否有进度，防止无限循环
        if (parser->hadError)
        {
            // 如果有错误，尝试恢复到下一个语句
            synchronize(parser);

            // 如果没有进度且未到达文件末尾，强制前进
            if (currentPos == parser->current && !isAtEnd(parser))
            {
                advance(parser);
            }
        }
        else if (currentPos == parser->current && !isAtEnd(parser))
        {
            // 即使没有错误，如果没有进度也要防止无限循环
            error(parser, "Unexpected token in block statement.");
            advance(parser);
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
Stmt *ifStatement(Parser *parser)
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
Stmt *whileStatement(Parser *parser)
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
Stmt *forStatement(Parser *parser)
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
Stmt *returnStatement(Parser *parser)
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

// 解析 switch 语句
Stmt *switchStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'switch'.");
    if (parser->hadError)
        return NULL;

    Expr *discriminant = expression(parser);
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after switch expression.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    consume(parser, TOKEN_LBRACE, "Expect '{' before switch body.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    // 解析 case 语句
    int capacity = 8;
    CaseStmt *cases = (CaseStmt *)malloc(capacity * sizeof(CaseStmt));
    int caseCount = 0;

    while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        if (caseCount >= capacity)
        {
            capacity *= 2;
            cases = (CaseStmt *)realloc(cases, capacity * sizeof(CaseStmt));
        }

        if (match(parser, TOKEN_CASE))
        {
            Expr *caseValue = expression(parser);
            if (parser->hadError)
            {
                freeExpr(discriminant);
                free(cases);
                return NULL;
            }

            consume(parser, TOKEN_COLON, "Expect ':' after case value.");
            if (parser->hadError)
            {
                freeExpr(discriminant);
                freeExpr(caseValue);
                free(cases);
                return NULL;
            }

            Stmt **caseStatements = NULL;
            int stmtCapacity = 4;
            int stmtCount = 0;
            caseStatements = (Stmt **)malloc(stmtCapacity * sizeof(Stmt *));

            // 解析直到遇到 case、default 或 }
            while (!check(parser, TOKEN_CASE) && !check(parser, TOKEN_DEFAULT) &&
                   !check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
            {
                if (stmtCount >= stmtCapacity)
                {
                    stmtCapacity *= 2;
                    caseStatements = (Stmt **)realloc(caseStatements, stmtCapacity * sizeof(Stmt *));
                }

                Stmt *stmt = statement(parser);
                if (parser->hadError)
                {
                    // 清理内存
                    for (int i = 0; i < stmtCount; i++)
                    {
                        freeStmt(caseStatements[i]);
                    }
                    free(caseStatements);
                    freeExpr(discriminant);
                    freeExpr(caseValue);
                    free(cases);
                    return NULL;
                }
                caseStatements[stmtCount++] = stmt;
            }

            // 创建块语句
            Stmt *caseBody = createBlockStmt(caseStatements, stmtCount);

            cases[caseCount].value = caseValue;
            cases[caseCount].body = caseBody;
            caseCount++;
        }
        else if (match(parser, TOKEN_DEFAULT))
        {
            consume(parser, TOKEN_COLON, "Expect ':' after 'default'.");
            if (parser->hadError)
            {
                freeExpr(discriminant);
                free(cases);
                return NULL;
            }

            // 类似地处理 default 语句体
            Stmt **defaultStatements = NULL;
            int stmtCapacity = 4;
            int stmtCount = 0;
            defaultStatements = (Stmt **)malloc(stmtCapacity * sizeof(Stmt *));

            while (!check(parser, TOKEN_CASE) && !check(parser, TOKEN_DEFAULT) &&
                   !check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
            {
                if (stmtCount >= stmtCapacity)
                {
                    stmtCapacity *= 2;
                    defaultStatements = (Stmt **)realloc(defaultStatements, stmtCapacity * sizeof(Stmt *));
                }

                Stmt *stmt = statement(parser);
                if (parser->hadError)
                {
                    for (int i = 0; i < stmtCount; i++)
                    {
                        freeStmt(defaultStatements[i]);
                    }
                    free(defaultStatements);
                    freeExpr(discriminant);
                    free(cases);
                    return NULL;
                }
                defaultStatements[stmtCount++] = stmt;
            }

            Stmt *defaultBody = createBlockStmt(defaultStatements, stmtCount);

            cases[caseCount].value = NULL; // NULL 表示 default
            cases[caseCount].body = defaultBody;
            caseCount++;
        }
        else
        {
            error(parser, "Expect 'case' or 'default' in switch statement.");
            freeExpr(discriminant);
            free(cases);
            return NULL;
        }
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after switch body.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        free(cases);
        return NULL;
    }

    return createSwitchStmt(discriminant, cases, caseCount);
}

// 解析 break 语句
Stmt *breakStatement(Parser *parser)
{
    Token keyword = previous(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'break'.");
    if (parser->hadError)
        return NULL;

    return createBreakStmt(keyword);
}

Stmt *doWhileStatement(Parser *parser)
{
    // 解析循环体
    Stmt *body = statement(parser);
    if (parser->hadError)
    {
        return NULL;
    }

    // 期望 while 关键字
    consume(parser, TOKEN_WHILE, "Expect 'while' after do body.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 期望左括号
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 调试：检查当前 token
    if (isAtEnd(parser))
    {
        error(parser, "Unexpected end of file in do-while condition.");
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 解析条件表达式
    Expr *condition = expression(parser);
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    if (condition == NULL)
    {
        error(parser, "Failed to parse do-while condition expression.");
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 期望右括号
    consume(parser, TOKEN_RPAREN, "Expect ')' after do-while condition.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    // 期望分号
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after do-while statement.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    return createDoWhileStmt(body, condition);
}
