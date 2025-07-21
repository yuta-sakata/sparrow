#ifndef SPARROW_PARSER_CORE_H
#define SPARROW_PARSER_CORE_H

#include "../lexer.h"
#include "../ast.h"

// 解析器状态
typedef struct
{
    Token *tokens;      // 令牌数组
    int current;        // 当前令牌索引
    int count;          // 令牌数量
    int hadError;       // 是否有解析错误
    char errorMsg[256]; // 解析错误信息
} Parser;

// 核心解析器函数
void initParser(Parser *parser, Token *tokens, int count);
Stmt **parse(Parser *parser, int *stmtCount);
int hadParseError(Parser *parser);
const char *getParseErrorMsg(Parser *parser);

// 辅助函数
int match(Parser *parser, TokenType type);
int check(Parser *parser, TokenType type);
Token advance(Parser *parser);
Token peek(Parser *parser);
Token previous(Parser *parser);
int isAtEnd(Parser *parser);
Token consume(Parser *parser, TokenType type, const char *message);
void synchronize(Parser *parser);
void error(Parser *parser, const char *message);

#endif // SPARROW_PARSER_CORE_H
