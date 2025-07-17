#ifndef SPARROW_PARSER_H
#define SPARROW_PARSER_H

#include "lexer.h"
#include "ast.h"

// 解析器状态
typedef struct
{
    Token *tokens;      // 令牌数组
    int current;        // 当前令牌索引
    int count;          // 令牌数量
    int hadError;       // 是否有解析错误
    char errorMsg[256]; // 解析错误信息
} Parser;

void initParser(Parser *parser, Token *tokens, int count); // 初始化解析器
Stmt **parse(Parser *parser, int *stmtCount);              // 解析语句
int hadParseError(Parser *parser);                         // 检查是否有解析错误
const char *getParseErrorMsg(Parser *parser);              // 获取解析错误信息

#endif // SPARROW_PARSER_H