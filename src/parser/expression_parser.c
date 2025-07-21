#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/parser/expression_parser.h"

// 解析表达式
Expr *expression(Parser *parser)
{
    return assignment(parser);
}

// 解析赋值表达式
Expr *assignment(Parser *parser)
{
    Expr *expr = logicalOr(parser);

    if (expr == NULL)
    {
        return NULL; // 直接返回NULL，不设置错误消息
    }

    if (match(parser, TOKEN_ASSIGN))
    {
        Expr *value = assignment(parser);
        if (value == NULL)
        {
            freeExpr(expr);
            return NULL;
        }

        if (expr->type == EXPR_VARIABLE)
        {
            Token name = expr->as.variable.name;
            freeExpr(expr);
            return createAssignExpr(name, value);
        }
        else if (expr->type == EXPR_ARRAY_ACCESS)
        {
            // 数组元素赋值
            return createArrayAssignExpr(expr->as.arrayAccess.array,
                                         expr->as.arrayAccess.index, value);
        }
        else if (expr->type == EXPR_DOT_ACCESS)
        {
            // 结构体字段赋值
            return createStructAssignExpr(expr->as.dotAccess.object,
                                          expr->as.dotAccess.member, value);
        }

        error(parser, "Invalid assignment target.");
        freeExpr(expr);
        freeExpr(value);
        return NULL;
    }

    return expr;
}

Expr *logicalOr(Parser *parser)
{
    Expr *expr = logicalAnd(parser);

    while (match(parser, TOKEN_OR))
    {
        TokenType operator = previous(parser).type;
        Expr *right = logicalAnd(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

Expr *logicalAnd(Parser *parser)
{
    Expr *expr = equality(parser);

    while (match(parser, TOKEN_AND))
    {
        TokenType operator = previous(parser).type;
        Expr *right = equality(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析相等性表达式
Expr *equality(Parser *parser)
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
Expr *comparison(Parser *parser)
{
    Expr *expr = term(parser);

    while (match(parser, TOKEN_LT) || match(parser, TOKEN_LE) ||
           match(parser, TOKEN_GT) || match(parser, TOKEN_GE) ||
           match(parser, TOKEN_IN))
    {
        TokenType operator = previous(parser).type;
        Expr *right = term(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析加减表达式
Expr *term(Parser *parser)
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
Expr *factor(Parser *parser)
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
Expr *unary(Parser *parser)
{
    // 检查类型转换：(type)expression
    if (match(parser, TOKEN_LPAREN))
    {
        // 检查是否为类型转换
        BaseType castType = TYPE_ANY;
        bool isCast = false;

        if (check(parser, TOKEN_INT))
        {
            castType = TYPE_INT;
            isCast = true;
        }
        else if (check(parser, TOKEN_FLOAT_TYPE))
        {
            castType = TYPE_FLOAT;
            isCast = true;
        }
        else if (check(parser, TOKEN_DOUBLE))
        {
            castType = TYPE_DOUBLE;
            isCast = true;
        }
        else if (check(parser, TOKEN_STRING_TYPE))
        {
            castType = TYPE_STRING;
            isCast = true;
        }
        else if (check(parser, TOKEN_BOOL))
        {
            castType = TYPE_BOOL;
            isCast = true;
        }

        if (isCast)
        {
            advance(parser); // 消费类型 token
            consume(parser, TOKEN_RPAREN, "Expect ')' after cast type.");
            if (parser->hadError)
                return NULL;

            Expr *expression = unary(parser);
            if (parser->hadError)
            {
                if (expression)
                    freeExpr(expression);
                return NULL;
            }

            return createCastExpr(castType, expression);
        }
        else
        {
            // 不是类型转换，退回并按分组表达式处理
            parser->current--; // 退回 '('

            // 继续处理其他一元表达式
            if (match(parser, TOKEN_NOT) || match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS))
            {
                TokenType operator = previous(parser).type;
                Expr *right = unary(parser);
                if (parser->hadError)
                {
                    if (right)
                        freeExpr(right);
                    return NULL;
                }
                return createUnaryExpr(operator, right);
            }
        }
    }

    if (match(parser, TOKEN_NOT) || match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        if (parser->hadError)
        {
            if (right)
                freeExpr(right);
            return NULL;
        }
        return createUnaryExpr(operator, right);
    }

    // 处理前缀运算符
    if (match(parser, TOKEN_PLUS_PLUS) || match(parser, TOKEN_MINUS_MINUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        if (parser->hadError)
        {
            if (right)
                freeExpr(right);
            return NULL;
        }

        // 检查右操作数是否是变量
        if (right == NULL || right->type != EXPR_VARIABLE)
        {
            error(parser, "Prefix operators can only be applied to variables.");
            if (right)
                freeExpr(right);
            return NULL;
        }

        return createPrefixExpr(right, operator);
    }

    return call(parser);
}

// 解析调用表达式
Expr *call(Parser *parser)
{
    Expr *expr = primary(parser);

    while (true)
    {
        if (match(parser, TOKEN_LPAREN))
        {
            expr = finishCall(parser, expr);
        }
        else if (match(parser, TOKEN_LBRACKET))
        {
            // 数组索引访问
            Expr *index = expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expect ']' after array index.");
            if (parser->hadError)
            {
                freeExpr(expr);
                freeExpr(index);
                return NULL;
            }
            expr = createArrayAccessExpr(expr, index);
        }
        else if (match(parser, TOKEN_DOT))
        {
            // 点访问（如枚举成员访问）
            Token member = consume(parser, TOKEN_IDENTIFIER, "Expect member name after '.'.");
            if (parser->hadError)
            {
                freeExpr(expr);
                return NULL;
            }
            expr = createDotAccessExpr(expr, member);
        }
        else if (match(parser, TOKEN_LBRACE))
        {
            // 结构体字面量语法：StructName { field1: value1, field2: value2 }
            if (expr->type != EXPR_VARIABLE)
            {
                error(parser, "Expected struct name before '{'.");
                freeExpr(expr);
                return NULL;
            }
            
            Token structName = expr->as.variable.name;
            freeExpr(expr); // 释放变量表达式，因为我们要创建结构体字面量
            
            // 解析字段初始化列表
            StructFieldInit *fields = NULL;
            int fieldCount = 0;
            int capacity = 0;
            
            if (!check(parser, TOKEN_RBRACE))
            {
                do
                {
                    // 扩展容量
                    if (fieldCount >= capacity)
                    {
                        capacity = capacity == 0 ? 4 : capacity * 2;
                        fields = (StructFieldInit *)realloc(fields, capacity * sizeof(StructFieldInit));
                        if (fields == NULL)
                        {
                            error(parser, "Memory allocation failed.");
                            return NULL;
                        }
                    }
                    
                    // 解析字段名
                    Token fieldName = consume(parser, TOKEN_IDENTIFIER, "Expect field name.");
                    if (parser->hadError)
                    {
                        if (fields) free(fields);
                        return NULL;
                    }
                    
                    // 期望冒号
                    consume(parser, TOKEN_COLON, "Expect ':' after field name.");
                    if (parser->hadError)
                    {
                        if (fields) free(fields);
                        return NULL;
                    }
                    
                    // 解析字段值
                    Expr *fieldValue = expression(parser);
                    if (parser->hadError)
                    {
                        if (fields) free(fields);
                        return NULL;
                    }
                    
                    // 添加字段
                    fields[fieldCount].name = fieldName;
                    fields[fieldCount].value = fieldValue;
                    fieldCount++;
                    
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RBRACE, "Expect '}' after struct fields.");
            if (parser->hadError)
            {
                // 清理内存
                for (int i = 0; i < fieldCount; i++)
                {
                    freeExpr(fields[i].value);
                }
                if (fields) free(fields);
                return NULL;
            }
            
            expr = createStructLiteralExpr(structName, fields, fieldCount);
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
            // 检查 createPostfixExpr 是否返回 NULL
            if (expr == NULL)
            {
                return NULL;
            }
        }
        else
        {
            break;
        }
    }

    return expr;
}

// 完成函数调用的解析
Expr *finishCall(Parser *parser, Expr *callee)
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

// 解析数组字面量
Expr *arrayLiteral(Parser *parser)
{
    Expr **elements = NULL;
    int count = 0;
    int capacity = 0;

    if (!check(parser, TOKEN_RBRACKET))
    {
        do
        {
            if (count >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                elements = (Expr **)realloc(elements, capacity * sizeof(Expr *));
            }

            elements[count++] = expression(parser);

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RBRACKET, "Expect ']' after array elements.");
    if (parser->hadError)
    {
        for (int i = 0; i < count; i++)
        {
            freeExpr(elements[i]);
        }
        free(elements);
        return NULL;
    }

    return createArrayLiteralExpr(elements, count);
}

// 解析基本表达式
Expr *primary(Parser *parser)
{
    if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_STRING))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        return createVariableExpr(previous(parser));
    }

    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_NULL))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_LBRACKET))
    {
        return arrayLiteral(parser);
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

    if (parser->hadError)
    {
        return NULL;
    }

    error(parser, "Expect expression.");
    return NULL;
}
