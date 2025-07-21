#ifndef SPARROW_EXPRESSION_PARSER_H
#define SPARROW_EXPRESSION_PARSER_H

#include "parser_core.h"

// 表达式解析函数
Expr *expression(Parser *parser);
Expr *assignment(Parser *parser);
Expr *logicalOr(Parser *parser);
Expr *logicalAnd(Parser *parser);
Expr *equality(Parser *parser);
Expr *comparison(Parser *parser);
Expr *term(Parser *parser);
Expr *factor(Parser *parser);
Expr *unary(Parser *parser);
Expr *call(Parser *parser);
Expr *primary(Parser *parser);
Expr *finishCall(Parser *parser, Expr *callee);
Expr *arrayLiteral(Parser *parser);

#endif // SPARROW_EXPRESSION_PARSER_H
