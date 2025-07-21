#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/parser/parser_core.h"
#include "../include/parser/declaration_parser.h"

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
        else if (stmt == NULL && currentPos == parser->current && !isAtEnd(parser))
        {
            // 即使没有错误，如果declaration返回NULL且没有进度，也要前进
            error(parser, "Failed to parse declaration.");
            advance(parser);
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

// 检查当前标记是否匹配指定类型
int match(Parser *parser, TokenType type)
{
    if (check(parser, type))
    {
        advance(parser);
        return 1;
    }
    return 0;
}

// 检查当前标记是否是指定类型
int check(Parser *parser, TokenType type)
{
    if (isAtEnd(parser))
        return 0;
    return peek(parser).type == type;
}

// 前进到下一个标记并返回它
Token advance(Parser *parser)
{
    if (!isAtEnd(parser))
        parser->current++;
    return previous(parser);
}

// 获取当前标记
Token peek(Parser *parser)
{
    return parser->tokens[parser->current];
}

// 获取上一个标记
Token previous(Parser *parser)
{
    return parser->tokens[parser->current - 1];
}

// 检查是否到达标记流末尾
int isAtEnd(Parser *parser)
{
    return peek(parser).type == TOKEN_EOF;
}

// 消费当前标记，如果类型匹配则前进，否则报错
Token consume(Parser *parser, TokenType type, const char *message)
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
void synchronize(Parser *parser)
{
    parser->hadError = 0; // 重置错误标志，尝试继续解析

    // 如果已经在文件末尾，直接返回
    if (isAtEnd(parser))
        return;

    advance(parser);

    while (!isAtEnd(parser))
    {
        if (previous(parser).type == TOKEN_SEMICOLON)
            return;

        switch (peek(parser).type)
        {
        case TOKEN_FUNCTION:
        case TOKEN_VAR:
        case TOKEN_CONST:
        case TOKEN_INT:
        case TOKEN_FLOAT_TYPE:
        case TOKEN_STRING_TYPE:
        case TOKEN_BOOL:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_FOR:
        case TOKEN_RETURN:
        case TOKEN_LBRACE:
        case TOKEN_RBRACE:
        case TOKEN_RBRACKET:
            return;
        default:
            break;
        }

        advance(parser);
    }
}

// 记录错误
void error(Parser *parser, const char *message)
{
    parser->hadError = 1;
    snprintf(parser->errorMsg, sizeof(parser->errorMsg), "Line %d: Error: %s",
             peek(parser).line, message);
}
