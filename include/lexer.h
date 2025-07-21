#ifndef SPARROW_LEXER_H
#define SPARROW_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 定义所有可能的标记类型
typedef enum
{
	TOKEN_EOF = 0,	  // 文件结束
	TOKEN_IDENTIFIER, // 标识符
	TOKEN_INTEGER,	  // 整数
	TOKEN_FLOAT,	  // 浮点数
	TOKEN_STRING,	  // 字符串

	// 运算符
	TOKEN_PLUS,		   // +
	TOKEN_MINUS,	   // -
	TOKEN_PLUS_PLUS,   // ++
	TOKEN_MINUS_MINUS, // --
	TOKEN_MULTIPLY,	   // *
	TOKEN_DIVIDE,	   // /
	TOKEN_MODULO,	   // %
	TOKEN_ASSIGN,	   // =
	TOKEN_EQ,		   // ==
	TOKEN_NE,		   // !=
	TOKEN_LT,		   // <
	TOKEN_LE,		   // <=
	TOKEN_GT,		   // >
	TOKEN_GE,		   // >=
	TOKEN_NOT,		   // !
	TOKEN_AND,		   // &&
	TOKEN_OR,		   // ||

	// 分隔符
	TOKEN_LPAREN,	 // (
	TOKEN_RPAREN,	 // )
	TOKEN_LBRACE,	 // {
	TOKEN_RBRACE,	 // }
	TOKEN_LBRACKET,	 // [
	TOKEN_RBRACKET,	 // ]
	TOKEN_SEMICOLON, // ;
	TOKEN_COMMA,	 // ,
	TOKEN_COLON,	 // :
	TOKEN_DOT,		 // .

	// 关键字
	TOKEN_IF,		   // if
	TOKEN_ELSE,		   // else
	TOKEN_IN,		   // in
	TOKEN_WHILE,	   // while
	TOKEN_FOR,		   // for
	TOKEN_RETURN,	   // return
	TOKEN_FUNCTION,	   // function
	TOKEN_VAR,		   // var
	TOKEN_VOID,		   // void
	TOKEN_INT,		   // int
	TOKEN_FLOAT_TYPE,  // float
	TOKEN_DOUBLE,	   // double
	TOKEN_STRING_TYPE, // string
	TOKEN_BOOL,		   // bool
	TOKEN_CONST,	   // const
	TOKEN_STATIC,	   // static
	TOKEN_SWITCH,	   // switch
	TOKEN_CASE,		   // case
	TOKEN_DEFAULT,	   // default
	TOKEN_DO,		   // do
	TOKEN_BREAK,	   // break
	TOKEN_IMPORT,	   // import
	TOKEN_NULL,		   // null
	TOKEN_TRUE,		   // true
	TOKEN_FALSE,	   // false
	TOKEN_ENUM,		   // enum
	TOKEN_STRUCT,	   // struct
	// 错误标记
	TOKEN_ERROR
} TokenType;

// 标记结构
typedef struct
{
	TokenType type;
	char *lexeme; // 标记文本
	int line;	  // 行号
	union
	{
		int intValue;	   // 整数值
		double floatValue; // 浮点数值
		char *stringValue; // 字符串值
	} value;
} Token;

// 词法分析器结构
typedef struct
{
	const char *source;	 // 源代码
	const char *current; // 当前解析位置
	int line;			 // 当前行号
} Lexer;

void initLexer(Lexer *lexer, const char *source);					// 初始化词法分析器
Token nextToken(Lexer *lexer);										// 获取下一个标记
void freeToken(Token *token);										// 释放标记内存
const char *getTokenName(TokenType type);							// 获取标记名称
Token *performLexicalAnalysis(const char *source, int *tokenCount); // 执行词法分析

#endif // SPARROW_LEXER_H