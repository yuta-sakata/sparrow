#ifndef SPARROW_PARSER_H
#define SPARROW_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct
{
    Token *tokens;
    int current;
    int count;
    int hadError;
    char errorMsg[256];
} Parser;

// 初始化语法分析器
void initParser(Parser *parser, Token *tokens, int count);

// 解析整个程序，返回语句列表
Stmt **parse(Parser *parser, int *stmtCount);

// 检查是否有语法错误
int hadParseError(Parser *parser);

// 获取错误信息
const char *getParseErrorMsg(Parser *parser);

#endif // SPARROW_PARSER_H