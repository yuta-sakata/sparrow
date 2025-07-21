#ifndef SPARROW_STATEMENT_PARSER_H
#define SPARROW_STATEMENT_PARSER_H

#include "parser_core.h"

// 语句解析函数
Stmt *statement(Parser *parser);
Stmt *expressionStatement(Parser *parser);
Stmt *blockStatement(Parser *parser);
Stmt *ifStatement(Parser *parser);
Stmt *whileStatement(Parser *parser);
Stmt *forStatement(Parser *parser);
Stmt *returnStatement(Parser *parser);
Stmt *switchStatement(Parser *parser);
Stmt *breakStatement(Parser *parser);
Stmt *doWhileStatement(Parser *parser);

#endif // SPARROW_STATEMENT_PARSER_H
