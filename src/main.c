#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "file_utils.h"

void executeProgram(Stmt **statements, int stmtCount)
{
	Interpreter interpreter;
	initInterpreter(&interpreter);

	// 执行程序
	interpret(&interpreter, statements, stmtCount);

	// 检查是否有运行时错误
	if (hadInterpreterError(&interpreter))
	{
		fprintf(stderr, "Runtime error: %s\n", getInterpreterError(&interpreter));
	}

	// 释放解释器资源
	freeInterpreter(&interpreter);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: sparrow [script]\n");
		return 1;
	}

	// 读取源代码文件
	char *source = readFile(argv[1]);
	if (source == NULL)
	{
		printf("Could not read file '%s'\n", argv[1]);
		return 1;
	}

	// 执行词法分析 (不输出过程)
	int tokenCount = 0;
	Token *tokens = performLexicalAnalysis(source, &tokenCount);
	if (tokens == NULL)
	{
		printf("Lexical analysis failed\n");
		free(source);
		return 1;
	}

	// 执行语法分析 (不输出过程)
	Parser parser;
	initParser(&parser, tokens, tokenCount);

	int stmtCount = 0;
	Stmt **statements = parse(&parser, &stmtCount);

	if (hadParseError(&parser))
	{
		printf("Parse error: %s\n", getParseErrorMsg(&parser));
	}
	else
	{
		// 执行程序
		executeProgram(statements, stmtCount);

		// 释放语句内存
		for (int i = 0; i < stmtCount; i++)
		{
			freeStmt(statements[i]);
		}
		free(statements);
	}

	// 释放标记和源代码内存
	for (int i = 0; i < tokenCount; i++)
	{
		freeToken(&tokens[i]);
	}
	free(tokens);
	free(source);

	return 0;
}