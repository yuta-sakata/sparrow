#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "file_utils.h"

/**
 * 执行程序语句
 * 
 * 该函数负责执行一系列解析后的语句。它会创建并初始化一个解释器实例，
 * 执行所有传入的语句，检查运行时错误，并在完成后清理资源。
 * 
 * @param statements 指向语句指针数组的指针，包含要执行的所有语句
 * @param stmtCount 语句数组中语句的数量
 * 
 * @note 如果在执行过程中发生运行时错误，错误信息将输出到stderr
 * @note 函数会自动管理解释器的生命周期，包括初始化和资源释放
 */
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