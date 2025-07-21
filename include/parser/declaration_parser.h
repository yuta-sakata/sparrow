#ifndef SPARROW_DECLARATION_PARSER_H
#define SPARROW_DECLARATION_PARSER_H

#include "parser_core.h"

// 声明解析函数
Stmt *declaration(Parser *parser);
Stmt *functionDeclaration(Parser *parser);
Stmt *varDeclaration(Parser *parser);
Stmt *constDeclaration(Parser *parser);
Stmt *enumDeclaration(Parser *parser);
Stmt *structDeclaration(Parser *parser);

#endif // SPARROW_DECLARATION_PARSER_H
