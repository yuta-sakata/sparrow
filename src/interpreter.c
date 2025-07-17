#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "interpreter.h"
#include "native_functions.h"

#include <math.h>

// 静态函数声明
static Value evaluateUnary(Interpreter *interpreter, Expr *expr);
static Value evaluateBinary(Interpreter *interpreter, Expr *expr);
static Value evaluateCall(Interpreter *interpreter, Expr *expr);
static Value evaluateLiteral(Expr *expr);
static Value evaluateGrouping(Interpreter *interpreter, Expr *expr);
static Value evaluateVariable(Interpreter *interpreter, Expr *expr);
static Value evaluateAssign(Interpreter *interpreter, Expr *expr);
static Value evaluatePostfix(Interpreter *interpreter, Expr *expr);
static Value evaluatePrefix(Interpreter *interpreter, Expr *expr);
static Value evaluateArrayLiteral(Interpreter *interpreter, Expr *expr);
static Value evaluateArrayAccess(Interpreter *interpreter, Expr *expr);
static Value evaluateArrayAssign(Interpreter *interpreter, Expr *expr);

static void executeExpression(Interpreter *interpreter, Stmt *stmt);
static void executeVar(Interpreter *interpreter, Stmt *stmt);
static void executeConst(Interpreter *interpreter, Stmt *stmt);
static void executeMultiVar(Interpreter *interpreter, Stmt *stmt);
static void executeBlock(Interpreter *interpreter, Stmt **statements, int count, Environment *environment);
static void executeIf(Interpreter *interpreter, Stmt *stmt);
static void executeWhile(Interpreter *interpreter, Stmt *stmt);
static void executeFor(Interpreter *interpreter, Stmt *stmt);
static void executeFunction(Interpreter *interpreter, Stmt *stmt);
static void executeReturn(Interpreter *interpreter, Stmt *stmt);
static void executeSwitch(Interpreter *interpreter, Stmt *stmt);
static void executeBreak(Interpreter *interpreter, Stmt *stmt);

static Value callFunction(Interpreter *interpreter, Function *function, Value *arguments, int argCount);
static void runtimeError(Interpreter *interpreter, const char *format, ...);

typedef struct
{
    bool hasBreak;
} BreakStatus;

typedef struct
{
	bool hasReturn;
	Value value;
} ReturnStatus;

// 静态变量
static ReturnStatus returnStatus;
static BreakStatus breakStatus = {false};

// 初始化解释器
void initInterpreter(Interpreter *interpreter)
{
	interpreter->globals = (Environment *)malloc(sizeof(Environment));
	initEnvironment(interpreter->globals, NULL);
	interpreter->environment = interpreter->globals;
	interpreter->hadError = false;
	interpreter->errorMessage[0] = '\0';
	interpreter->hasMainFunction = false; // 初始化为 false
	interpreter->mainFunction = NULL;	  // 初始化为 NULL

	registerAllNativeFunctions(interpreter);
}

void interpret(Interpreter *interpreter, Stmt **statements, int count)
{
	// 第一阶段：只执行函数定义
	for (int i = 0; i < count; i++)
	{
		if (statements[i]->type == STMT_FUNCTION)
		{
			execute(interpreter, statements[i]);
			if (interpreter->hadError)
			{
				return;
			}
		}
	}

	// 第二阶段：执行其他语句
	for (int i = 0; i < count; i++)
	{
		if (statements[i]->type != STMT_FUNCTION)
		{
			execute(interpreter, statements[i]);
			if (interpreter->hadError)
				return;
		}
	}

	// 如果找到了 main 函数，自动调用它
	if (interpreter->hasMainFunction && interpreter->mainFunction != NULL)
	{
		Value *noArgs = NULL;
		Value result = callFunction(interpreter, interpreter->mainFunction, noArgs, 0);
		freeValue(result);
	}
}

// 执行语句
void execute(Interpreter *interpreter, Stmt *stmt)
{
	if (stmt == NULL)
		return;

	switch (stmt->type)
	{
	case STMT_EXPRESSION:
		executeExpression(interpreter, stmt);
		break;
	case STMT_VAR:
		executeVar(interpreter, stmt);
		break;
	case STMT_CONST:
		executeConst(interpreter, stmt);
		break;
	case STMT_MULTI_VAR:
		executeMultiVar(interpreter, stmt);
		break;
	case STMT_BLOCK:
		executeBlock(interpreter, stmt->as.block.statements,
					 stmt->as.block.count,
					 interpreter->environment);
		break;
	case STMT_IF:
		executeIf(interpreter, stmt);
		break;
	case STMT_WHILE:
		executeWhile(interpreter, stmt);
		break;
	case STMT_FOR:
		executeFor(interpreter, stmt);
		break;
	case STMT_FUNCTION:
		executeFunction(interpreter, stmt);
		break;
	case STMT_RETURN:
		executeReturn(interpreter, stmt);
		break;
	case STMT_SWITCH:
        executeSwitch(interpreter, stmt);
        break;
    case STMT_BREAK:
        executeBreak(interpreter, stmt);
        break;
	}
}

// 实现表达式求值
Value evaluate(Interpreter *interpreter, Expr *expr)
{
	if (expr == NULL)
	{
		printf("ERROR: Trying to evaluate NULL expression\n");
		return createNull();
	}

	switch (expr->type)
	{
	case EXPR_LITERAL:
		return evaluateLiteral(expr);
	case EXPR_GROUPING:
		return evaluateGrouping(interpreter, expr);
	case EXPR_UNARY:
		return evaluateUnary(interpreter, expr);
	case EXPR_BINARY:
		return evaluateBinary(interpreter, expr);
	case EXPR_VARIABLE:
		return evaluateVariable(interpreter, expr);
	case EXPR_ASSIGN:
		return evaluateAssign(interpreter, expr);
	case EXPR_CALL:
		return evaluateCall(interpreter, expr);
	case EXPR_POSTFIX:
		return evaluatePostfix(interpreter, expr);
	case EXPR_PREFIX:
		return evaluatePrefix(interpreter, expr);
	case EXPR_ARRAY_LITERAL:
		return evaluateArrayLiteral(interpreter, expr);
	case EXPR_ARRAY_ACCESS:
		return evaluateArrayAccess(interpreter, expr);
	case EXPR_ARRAY_ASSIGN:
		return evaluateArrayAssign(interpreter, expr);
	}

	return createNull();
}

// 运行时错误处理
static void runtimeError(Interpreter *interpreter, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(interpreter->errorMessage, sizeof(interpreter->errorMessage), format, args);
	va_end(args);

	interpreter->hadError = true;
}

// 执行表达式语句
static void executeExpression(Interpreter *interpreter, Stmt *stmt)
{
	Value value = evaluate(interpreter, stmt->as.expression.expression);
	// 表达式语句的结果通常被丢弃
	freeValue(value);
}

// 解析变量声明
static void executeVar(Interpreter *interpreter, Stmt *stmt)
{
	Value value = createNull();

	if (stmt->as.var.initializer != NULL)
	{
		value = evaluate(interpreter, stmt->as.var.initializer);
	}

	defineVariable(interpreter->environment, stmt->as.var.name.lexeme, value);
	freeValue(value);
}

static void executeConst(Interpreter *interpreter, Stmt *stmt)
{
	Value value = createNull();

	// 常量必须有初始值
	if (stmt->as.constStmt.initializer != NULL)
	{
		value = evaluate(interpreter, stmt->as.constStmt.initializer);
		if (interpreter->hadError)
		{
			freeValue(value);
			return;
		}
	}
	else
	{
		runtimeError(interpreter, "Constants must be initialized.");
		return;
	}

	// 定义常量
	defineConstant(interpreter->environment, stmt->as.constStmt.name.lexeme, value);

	freeValue(value);
}

// 解析多变量声明
static void executeMultiVar(Interpreter *interpreter, Stmt *stmt)
{
	// 评估初始值（如果有）
	Value initialValue = createNull();
	if (stmt->as.multiVar.initializer != NULL)
	{
		initialValue = evaluate(interpreter, stmt->as.multiVar.initializer);
	}

	// 为每个变量定义相同的值（复制初始值）
	for (int i = 0; i < stmt->as.multiVar.count; i++)
	{
		Value valueCopy = copyValue(initialValue);
		defineVariable(interpreter->environment, stmt->as.multiVar.names[i].lexeme, valueCopy);
		freeValue(valueCopy); // defineVariable会创建自己的副本
	}

	// 释放原始初始值
	freeValue(initialValue);
}

// 执行代码块
static void executeBlock(Interpreter *interpreter, Stmt **statements, int count, Environment *environment)
{
	Environment *previous = interpreter->environment;

	// 创建一个新的环境用于代码块
	Environment blockEnv;
	initEnvironment(&blockEnv, environment);

	interpreter->environment = &blockEnv;

	for (int i = 0; i < count; i++)
	{
		execute(interpreter, statements[i]);
		if (interpreter->hadError)
			break;
	}

	// 恢复之前的环境
	interpreter->environment = previous;

	// 释放块级环境
	freeEnvironment(&blockEnv);
}

// 实现一元表达式求值
static Value evaluateUnary(Interpreter *interpreter, Expr *expr)
{
	Value right = evaluate(interpreter, expr->as.unary.right);

	switch (expr->as.unary.op)
	{
	case TOKEN_MINUS: // 一元减法运算符 (-)
		if (right.type != VAL_NUMBER)
		{
			freeValue(right);
			runtimeError(interpreter, "操作数必须是数字。");
			return createNull();
		}
		{
			double value = -right.as.number;
			freeValue(right);
			return createNumber(value);
		}

	case TOKEN_PLUS: // 一元加法运算符 (+)
		if (right.type != VAL_NUMBER)
		{
			freeValue(right);
			runtimeError(interpreter, "操作数必须是数字。");
			return createNull();
		}
		return right;

	case TOKEN_NOT: // 逻辑非运算符 (!)
	{
		bool isTruthy = (right.type != VAL_NULL &&
						 !(right.type == VAL_BOOL && !right.as.boolean));
		freeValue(right);
		return createBool(!isTruthy);
	}
	default:
		// 如果不是已知的一元运算符
		freeValue(right);
		runtimeError(interpreter, "未知的一元运算符。");
		return createNull();
	}
}

static Value evaluateBinary(Interpreter *interpreter, Expr *expr)
{
	Value left = evaluate(interpreter, expr->as.binary.left);

	// 短路求值处理（对于逻辑运算符）
	if ((expr->as.binary.op == TOKEN_AND || expr->as.binary.op == TOKEN_OR) &&
		interpreter->hadError == false)
	{
		bool leftTruthy = left.type != VAL_NULL &&
						  !(left.type == VAL_BOOL && !left.as.boolean);

		if ((expr->as.binary.op == TOKEN_AND && !leftTruthy) ||
			(expr->as.binary.op == TOKEN_OR && leftTruthy))
		{
			return left; // 短路返回
		}

		freeValue(left);
		return evaluate(interpreter, expr->as.binary.right);
	}

	Value right = evaluate(interpreter, expr->as.binary.right);

	if (interpreter->hadError)
	{
		freeValue(left);
		freeValue(right);
		return createNull();
	}

	switch (expr->as.binary.op)
	{
	// 算术运算符
	case TOKEN_PLUS: // 连接运算符 (+)
		if (left.type == VAL_NUMBER && right.type == VAL_NUMBER)
		{
			double result = left.as.number + right.as.number;
			freeValue(left);
			freeValue(right);
			return createNumber(result);
		}
		else if (left.type == VAL_STRING && right.type == VAL_STRING)
		{
			// 字符串连接
			int leftLen = strlen(left.as.string);
			int rightLen = strlen(right.as.string);
			char *result = malloc(leftLen + rightLen + 1);

			if (result == NULL)
			{
				freeValue(left);
				freeValue(right);
				runtimeError(interpreter, "内存分配失败");
				return createNull();
			}

			strcpy(result, left.as.string);
			strcat(result, right.as.string);

			freeValue(left);
			freeValue(right);

			Value strValue = createString(result);
			free(result);
			return strValue;
		}

		freeValue(left);
		freeValue(right);
		runtimeError(interpreter, "+ 运算符只支持数字加法或字符串连接");
		return createNull();

	case TOKEN_MINUS: // 减法运算符 (-)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "- 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			double result = left.as.number - right.as.number;
			freeValue(left);
			freeValue(right);
			return createNumber(result);
		}

	case TOKEN_MULTIPLY: // 乘法运算符 (*)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "* 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			double result = left.as.number * right.as.number;
			freeValue(left);
			freeValue(right);
			return createNumber(result);
		}

	case TOKEN_DIVIDE: // 除法运算符 (/)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "/ 运算符的操作数必须是数字。");
			return createNull();
		}

		// 检查除数是否为零
		if (right.as.number == 0)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "除数不能为零。");
			return createNull();
		}

		{
			double result = left.as.number / right.as.number;
			freeValue(left);
			freeValue(right);
			return createNumber(result);
		}

	case TOKEN_MODULO: // 取模运算符 (%)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "%% 运算符的操作数必须是数字。");
			return createNull();
		}
		// 检查除数是否为零
		if (right.as.number == 0)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "取模运算的除数不能为零。");
			return createNull();
		}
		{
			double result = fmod(left.as.number, right.as.number);
			freeValue(left);
			freeValue(right);
			return createNumber(result);
		}
	case TOKEN_LT: // 小于运算符 (<)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "< 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			bool result = left.as.number < right.as.number;
			freeValue(left);
			freeValue(right);
			return createBool(result);
		}
	case TOKEN_LE: // 小于等于运算符 (<=)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "<= 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			bool result = left.as.number <= right.as.number;
			freeValue(left);
			freeValue(right);
			return createBool(result);
		}
	case TOKEN_GT: // 大于运算符 (>)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, "> 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			bool result = left.as.number > right.as.number;
			freeValue(left);
			freeValue(right);
			return createBool(result);
		}
	case TOKEN_GE: // 大于等于运算符 (>=)
		if (left.type != VAL_NUMBER || right.type != VAL_NUMBER)
		{
			freeValue(left);
			freeValue(right);
			runtimeError(interpreter, ">= 运算符的操作数必须是数字。");
			return createNull();
		}
		{
			bool result = left.as.number >= right.as.number;
			freeValue(left);
			freeValue(right);
			return createBool(result);
		}

	case TOKEN_EQ: // 等于运算符 (==)
	{
		bool result = valuesEqual(left, right);
		freeValue(left);
		freeValue(right);
		return createBool(result);
	}
	case TOKEN_NE: // 不相等运算符 (!=)
	{
		bool result = !valuesEqual(left, right);
		freeValue(left);
		freeValue(right);
		return createBool(result);
	}
	case TOKEN_AND: // 逻辑与运算符 (&&)
	{
		// 短路求值：如果左操作数为假，直接返回false
		if (left.type == VAL_BOOL && !left.as.boolean)
		{
			freeValue(left);
			freeValue(right);
			return createBool(false);
		}
		else if (left.type == VAL_NULL)
		{
			freeValue(left);
			freeValue(right);
			return createBool(false);
		}

		// 左操作数为真，返回右操作数的真值
		bool result = (right.type != VAL_NULL) &&
					  !(right.type == VAL_BOOL && !right.as.boolean);
		freeValue(left);
		freeValue(right);
		return createBool(result);
	}

	case TOKEN_OR: // 逻辑或运算符 (||)
	{
		// 短路求值：如果左操作数为真，直接返回true
		bool leftTruthy = (left.type != VAL_NULL) &&
						  !(left.type == VAL_BOOL && !left.as.boolean);

		if (leftTruthy)
		{
			freeValue(left);
			freeValue(right);
			return createBool(true);
		}

		// 左操作数为假，返回右操作数的真值
		bool result = (right.type != VAL_NULL) &&
					  !(right.type == VAL_BOOL && !right.as.boolean);
		freeValue(left);
		freeValue(right);
		return createBool(result);
	}
	}

	freeValue(left);
	freeValue(right);
	runtimeError(interpreter, "不支持的二元运算符");
	return createNull();
}

// 实现字面量表达式求值
static Value evaluateLiteral(Expr *expr)
{
	Token token = expr->as.literal.value;

	switch (token.type)
	{
	case TOKEN_INTEGER:
	{
		double value = atof(token.lexeme);
		return createNumber(value);
	}
	case TOKEN_FLOAT:
	{
		double value = atof(token.lexeme);
		return createNumber(value);
	}
	case TOKEN_STRING:
	{
		// 去掉字符串两端的引号
		int len = strlen(token.lexeme);
		if (len >= 2 && token.lexeme[0] == '"' && token.lexeme[len - 1] == '"')
		{
			if (token.value.stringValue != NULL)
			{
				return createString(token.value.stringValue);
			}
			else
			{
				// fallback: 手动去掉引号
				char *str = malloc(len - 1);
				if (str != NULL)
				{
					strncpy(str, token.lexeme + 1, len - 2);
					str[len - 2] = '\0';
					Value result = createString(str);
					free(str);
					return result;
				}
			}
		}
		return createString(token.lexeme);
	}
	case TOKEN_TRUE:
		return createBool(true);
	case TOKEN_FALSE:
		return createBool(false);
	case TOKEN_NULL:
		return createNull();
	default:
		return createNull();
	}
}

// 实现分组表达式求值
static Value evaluateGrouping(Interpreter *interpreter, Expr *expr)
{
	return evaluate(interpreter, expr->as.grouping.expression);
}

// 实现变量表达式求值
static Value evaluateVariable(Interpreter *interpreter, Expr *expr)
{
	// 安全检查
	if (expr == NULL)
	{
		printf("ERROR: NULL expression in evaluateVariable\n");
		return createNull();
	}

	if (expr->as.variable.name.lexeme == NULL)
	{
		printf("ERROR: NULL variable name in evaluateVariable\n");
		return createNull();
	}

	Value result = getVariable(interpreter->environment, expr->as.variable.name);

	return result;
}
// 实现赋值表达式求值
static Value evaluateAssign(Interpreter *interpreter, Expr *expr)
{
	Value value = evaluate(interpreter, expr->as.assign.value);

	if (interpreter->hadError)
		return createNull();

	assignVariable(interpreter->environment, expr->as.assign.name, value);

	// 赋值表达式的值是被赋的值
	return value;
}

Value evaluatePostfix(Interpreter *interpreter, Expr *expr)
{
	// 确保操作数是变量
	if (expr->as.postfix.operand->type != EXPR_VARIABLE)
	{
		runtimeError(interpreter, "后缀运算符只能应用于变量。");
		return createNull();
	}

	// 获取变量的当前值
	Token varName = expr->as.postfix.operand->as.variable.name;
	Value oldValue = getVariable(interpreter->environment, varName);

	if (interpreter->hadError)
	{
		return createNull();
	}

	// 检查变量类型
	if (oldValue.type != VAL_NUMBER)
	{
		freeValue(oldValue);
		runtimeError(interpreter, "后缀运算符只能应用于数字类型。");
		return createNull();
	}

	// 计算新值
	Value newValue;
	if (expr->as.postfix.op == TOKEN_PLUS_PLUS)
	{
		newValue = createNumber(oldValue.as.number + 1);
	}
	else if (expr->as.postfix.op == TOKEN_MINUS_MINUS)
	{
		newValue = createNumber(oldValue.as.number - 1);
	}
	else
	{
		freeValue(oldValue);
		runtimeError(interpreter, "未知的后缀运算符。");
		return createNull();
	}

	// 更新变量值
	assignVariable(interpreter->environment, varName, newValue);
	freeValue(newValue);

	// 后缀运算符返回原来的值
	return oldValue;
}

Value evaluatePrefix(Interpreter *interpreter, Expr *expr)
{
	Token varName = expr->as.prefix.operand->as.variable.name;
	Value oldValue = getVariable(interpreter->environment, varName);

	if (interpreter->hadError)
	{
		return createNull();
	}

	// 检查变量类型
	if (oldValue.type != VAL_NUMBER)
	{
		freeValue(oldValue);
		runtimeError(interpreter, "前缀运算符只能应用于数字类型。");
		return createNull();
	}

	// 计算新值
	Value newValue;
	if (expr->as.prefix.op == TOKEN_PLUS_PLUS)
	{
		newValue = createNumber(oldValue.as.number + 1);
	}
	else if (expr->as.prefix.op == TOKEN_MINUS_MINUS)
	{
		newValue = createNumber(oldValue.as.number - 1);
	}
	else
	{
		freeValue(oldValue);
		runtimeError(interpreter, "未知的前缀运算符。");
		return createNull();
	}

	// 更新变量值
	assignVariable(interpreter->environment, varName, newValue);

	// 前缀运算符返回新值
	freeValue(oldValue);
	return newValue;
}

// 实现数组字面量求值
static Value evaluateArrayLiteral(Interpreter *interpreter, Expr *expr)
{
	Value arrayValue = createArray(TYPE_ANY, expr->as.arrayLiteral.elementCount);
	if (arrayValue.type == VAL_NULL)
	{
		runtimeError(interpreter, "无法创建数组");
		return createNull();
	}

	// 求值所有元素并添加到数组
	for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++)
	{
		Value element = evaluate(interpreter, expr->as.arrayLiteral.elements[i]);
		if (interpreter->hadError)
		{
			freeValue(arrayValue);
			return createNull();
		}

		// 创建元素的副本，确保内存安全
		Value elementCopy = copyValue(element);
		arrayPush(arrayValue.as.array, elementCopy);

		// 释放临时的element
		freeValue(element);
	}

	return arrayValue;
}

// 实现数组访问求值
static Value evaluateArrayAccess(Interpreter *interpreter, Expr *expr)
{
	Value arrayValue = evaluate(interpreter, expr->as.arrayAccess.array);
	if (interpreter->hadError)
	{
		return createNull();
	}

	Value indexValue = evaluate(interpreter, expr->as.arrayAccess.index);
	if (interpreter->hadError)
	{
		freeValue(arrayValue);
		return createNull();
	}

	// 检查数组类型
	if (arrayValue.type != VAL_ARRAY)
	{
		freeValue(arrayValue);
		freeValue(indexValue);
		runtimeError(interpreter, "只能对数组进行索引访问");
		return createNull();
	}

	// 检查索引类型
	if (indexValue.type != VAL_NUMBER)
	{
		freeValue(arrayValue);
		freeValue(indexValue);
		runtimeError(interpreter, "数组索引必须是数字");
		return createNull();
	}

	int index = (int)indexValue.as.number;
	Value result = arrayGet(arrayValue.as.array, index);

	freeValue(arrayValue);
	freeValue(indexValue);

	return result;
}

// 实现数组赋值求值
static Value evaluateArrayAssign(Interpreter *interpreter, Expr *expr)
{
	if (expr->as.arrayAssign.array->type == EXPR_VARIABLE)
	{
		Token arrayName = expr->as.arrayAssign.array->as.variable.name;

		// 从环境中获取数组引用
		Value *arrayRef = getVariableRef(interpreter->environment, arrayName.lexeme);
		if (arrayRef == NULL || arrayRef->type != VAL_ARRAY)
		{
			runtimeError(interpreter, "变量不是数组或不存在");
			return createNull();
		}

		Value indexValue = evaluate(interpreter, expr->as.arrayAssign.index);
		if (interpreter->hadError)
			return createNull();

		Value value = evaluate(interpreter, expr->as.arrayAssign.value);
		if (interpreter->hadError)
		{
			freeValue(indexValue);
			return createNull();
		}

		if (indexValue.type != VAL_NUMBER)
		{
			freeValue(indexValue);
			freeValue(value);
			runtimeError(interpreter, "数组索引必须是数字");
			return createNull();
		}

		int index = (int)indexValue.as.number;
		arraySet(arrayRef->as.array, index, value);

		freeValue(indexValue);
		return value;
	}
	else
	{
		// 处理复杂表达式（如嵌套数组访问）
		Value arrayValue = evaluate(interpreter, expr->as.arrayAssign.array);
		if (interpreter->hadError)
			return createNull();

		Value indexValue = evaluate(interpreter, expr->as.arrayAssign.index);
		if (interpreter->hadError)
		{
			freeValue(arrayValue);
			return createNull();
		}

		Value value = evaluate(interpreter, expr->as.arrayAssign.value);
		if (interpreter->hadError)
		{
			freeValue(arrayValue);
			freeValue(indexValue);
			return createNull();
		}

		if (arrayValue.type != VAL_ARRAY)
		{
			freeValue(arrayValue);
			freeValue(indexValue);
			freeValue(value);
			runtimeError(interpreter, "只能对数组进行索引赋值");
			return createNull();
		}

		if (indexValue.type != VAL_NUMBER)
		{
			freeValue(arrayValue);
			freeValue(indexValue);
			freeValue(value);
			runtimeError(interpreter, "数组索引必须是数字");
			return createNull();
		}

		int index = (int)indexValue.as.number;
		arraySet(arrayValue.as.array, index, value);

		freeValue(arrayValue);
		freeValue(indexValue);
		return value;
	}
}

// 实现条件语句执行
static void executeIf(Interpreter *interpreter, Stmt *stmt)
{
	Value condition = evaluate(interpreter, stmt->as.ifStmt.condition);

	// 检查条件是否为真
	bool isTruthy = condition.type != VAL_NULL &&
					!(condition.type == VAL_BOOL && !condition.as.boolean);

	freeValue(condition);

	if (isTruthy)
	{
		execute(interpreter, stmt->as.ifStmt.thenBranch);
	}
	else if (stmt->as.ifStmt.elseBranch != NULL)
	{
		execute(interpreter, stmt->as.ifStmt.elseBranch);
	}
}

// 实现while循环语句执行
static void executeWhile(Interpreter *interpreter, Stmt *stmt)
{
	while (true)
	{
		Value condition = evaluate(interpreter, stmt->as.whileLoop.condition);

		// 检查条件是否为真
		bool isTruthy = condition.type != VAL_NULL &&
						!(condition.type == VAL_BOOL && !condition.as.boolean);

		freeValue(condition);

		if (!isTruthy)
			break;

		execute(interpreter, stmt->as.whileLoop.body);

		if (interpreter->hadError)
			break;
	}
}

// 实现for循环语句执行
static void executeFor(Interpreter *interpreter, Stmt *stmt)
{
	// 执行初始化语句
	if (stmt->as.forLoop.initializer != NULL)
	{
		execute(interpreter, stmt->as.forLoop.initializer);
	}

	while (true)
	{
		// 检查条件
		if (stmt->as.forLoop.condition != NULL)
		{
			Value condition = evaluate(interpreter, stmt->as.forLoop.condition);

			bool isTruthy = condition.type != VAL_NULL &&
							!(condition.type == VAL_BOOL && !condition.as.boolean);

			freeValue(condition);

			if (!isTruthy)
				break;
		}

		// 执行循环体
		execute(interpreter, stmt->as.forLoop.body);

		if (interpreter->hadError)
			break;

		// 执行增量表达式
		if (stmt->as.forLoop.increment != NULL)
		{
			Value incrementResult = evaluate(interpreter, stmt->as.forLoop.increment);
			freeValue(incrementResult);
		}
	}
}
static Value evaluateCall(Interpreter *interpreter, Expr *expr)
{
	// 安全检查
	if (expr == NULL || expr->as.call.callee == NULL)
	{
		printf("ERROR: NULL callee in function call\n");
		return createNull();
	}

	Value callee = evaluate(interpreter, expr->as.call.callee);

	if (interpreter->hadError)
	{
		freeValue(callee);
		return createNull();
	}

	// 收集参数
	Value *arguments = NULL;
	if (expr->as.call.argCount > 0)
	{
		arguments = malloc(sizeof(Value) * expr->as.call.argCount);

		if (arguments == NULL)
		{
			freeValue(callee);
			runtimeError(interpreter, "内存分配失败");
			return createNull();
		}

		for (int i = 0; i < expr->as.call.argCount; i++)
		{
			if (expr->as.call.arguments[i] == NULL)
			{
				printf("ERROR: NULL argument %d in function call\n", i);
				// 清理已分配内存
				for (int j = 0; j < i; j++)
				{
					freeValue(arguments[j]);
				}
				free(arguments);
				freeValue(callee);
				return createNull();
			}

			arguments[i] = evaluate(interpreter, expr->as.call.arguments[i]);
			// ... 其余代码
		}
	}

	// 根据被调用者类型进行调用
	Value result;
	if (callee.type == VAL_FUNCTION)
	{
		// 确保函数指针不为 NULL
		if (callee.as.function == NULL)
		{
			printf("ERROR: NULL function pointer\n");
			result = createNull();
		}
		else
		{
			// 调用用户定义函数
			result = callFunction(interpreter, callee.as.function, arguments, expr->as.call.argCount);
		}
	}
	else if (callee.type == VAL_NATIVE_FUNCTION)
	{
		// 确保原生函数指针不为 NULL
		if (callee.as.nativeFunction == NULL)
		{
			printf("ERROR: NULL native function pointer\n");
			result = createNull();
		}
		else
		{
			// 确保函数指针有效
			if (callee.as.nativeFunction->function != NULL)
			{
				// 调用原生函数
				result = callee.as.nativeFunction->function(expr->as.call.argCount, arguments);
			}
			else
			{
				printf("ERROR: Native function has NULL function pointer\n");
				result = createNull();
			}
		}
	}
	else
	{
		// 非可调用对象
		runtimeError(interpreter, "只能调用函数。");
		result = createNull();
	}

	// 清理资源
	for (int i = 0; i < expr->as.call.argCount; i++)
	{
		freeValue(arguments[i]);
	}

	free(arguments);
	freeValue(callee);

	return result;
}

/**
 * 执行函数定义语句，创建函数对象并将其绑定到当前环境
 *
 * 该函数负责：
 * 1. 创建并初始化函数对象
 * 2. 分配并复制函数名
 * 3. 设置函数参数数量和返回类型
 * 4. 分配并复制参数名数组
 * 5. 设置函数体语句
 * 6. 设置闭包环境（当前为全局环境）
 * 7. 检查是否为 main 函数并进行特殊处理
 * 8. 将函数对象包装为 Value 并定义到当前环境中
 *
 * 内存管理：
 * - 动态分配函数对象和相关字符串内存
 * - 在分配失败时进行完整的内存清理
 * - 参数名数组的分配采用逐个分配策略，失败时清理已分配部分
 *
 * @param interpreter 解释器实例，包含环境和全局状态
 * @param stmt 函数定义语句，包含函数名、参数、返回类型和函数体
 *
 * @note 如果内存分配失败，会调用 runtimeError 并提前返回
 * @note main 函数会被特殊标记并存储在解释器中
 */
static void executeFunction(Interpreter *interpreter, Stmt *stmt)
{
	// 创建函数对象
	Function *function = (Function *)malloc(sizeof(Function));
	if (function == NULL)
	{
		runtimeError(interpreter, "内存分配失败");
		return;
	}

	size_t nameLen = strlen(stmt->as.function.name.lexeme);
	function->name = (char *)malloc(nameLen + 1);
	if (function->name == NULL)
	{
		free(function);
		runtimeError(interpreter, "内存分配失败");
		return;
	}
	strcpy(function->name, stmt->as.function.name.lexeme);

	function->arity = stmt->as.function.paramCount;
	function->paramTypes = NULL;
	function->returnType = stmt->as.function.returnType;

	// 这里缺少重要的函数属性设置！
	// 需要设置参数名、函数体、闭包环境等

	// 分配参数名数组
	if (function->arity > 0)
	{
		function->paramNames = (char **)malloc(sizeof(char *) * function->arity);
		if (function->paramNames == NULL)
		{
			free(function->name);
			free(function);
			runtimeError(interpreter, "内存分配失败");
			return;
		}

		// 复制参数名
		for (int i = 0; i < function->arity; i++)
		{
			size_t paramNameLen = strlen(stmt->as.function.params[i].lexeme);
			function->paramNames[i] = (char *)malloc(paramNameLen + 1);
			if (function->paramNames[i] == NULL)
			{
				// 清理已分配的内存
				for (int j = 0; j < i; j++)
				{
					free(function->paramNames[j]);
				}
				free(function->paramNames);
				free(function->name);
				free(function);
				runtimeError(interpreter, "内存分配失败");
				return;
			}
			strcpy(function->paramNames[i], stmt->as.function.params[i].lexeme);
		}
	}
	else
	{
		function->paramNames = NULL;
	}

	// 设置函数体
	function->body = stmt->as.function.body;

	// 设置闭包环境（当前为全局环境）
	function->closure = interpreter->globals;

	// 检查是否是 main 函数
	if (strcmp(function->name, "main") == 0)
	{
		interpreter->hasMainFunction = true;
		interpreter->mainFunction = function;
	}

	Value functionValue = createFunction(function);
	defineVariable(interpreter->environment, function->name, functionValue);
}

// 执行返回语句
static void executeReturn(Interpreter *interpreter, Stmt *stmt)
{
	Value value = createNull();

	if (stmt->as.returnStmt.value != NULL)
	{
		value = evaluate(interpreter, stmt->as.returnStmt.value);
	}

	// 设置返回状态
	returnStatus.hasReturn = true;
	returnStatus.value = value;
}

static void executeSwitch(Interpreter *interpreter, Stmt *stmt)
{
    Value discriminant = evaluate(interpreter, stmt->as.switchStmt.discriminant);
    if (interpreter->hadError)
    {
        return;
    }

    bool matched = false;
    bool fallthrough = false;
    
    // 重置 break 状态
    breakStatus.hasBreak = false;

    for (int i = 0; i < stmt->as.switchStmt.caseCount; i++)
    {
        CaseStmt *caseStmt = &stmt->as.switchStmt.cases[i];
        
        if (caseStmt->value == NULL) // default case
        {
            if (!matched)
            {
                matched = true;
                fallthrough = true;
            }
        }
        else
        {
            Value caseValue = evaluate(interpreter, caseStmt->value);
            if (interpreter->hadError)
            {
                freeValue(discriminant);
                return;
            }

            if (!matched && valuesEqual(discriminant, caseValue))
            {
                matched = true;
                fallthrough = true;
            }
            
            freeValue(caseValue);
        }

        if (fallthrough)
        {
            execute(interpreter, caseStmt->body);
            
            if (breakStatus.hasBreak || interpreter->hadError)
            {
                break;
            }
        }
    }

    freeValue(discriminant);
    breakStatus.hasBreak = false; // 重置状态
}

// 执行 break 语句
static void executeBreak(Interpreter *interpreter, Stmt *stmt)
{
    breakStatus.hasBreak = true;
}
/**
 * 调用函数并执行函数体
 *
 * 此函数负责执行用户定义的函数调用，包括参数验证、环境设置、
 * 参数绑定和函数体执行。
 *
 * @param interpreter 解释器实例指针，用于执行函数体和错误处理
 * @param function 要调用的函数对象指针，包含函数的元数据和函数体
 * @param arguments 传入的参数值数组
 * @param argCount 传入参数的数量
 *
 * @return Value 函数的返回值。如果函数有显式返回语句则返回该值，
 *               否则返回null值。如果参数数量不匹配则返回null值。
 *
 * 函数执行流程：
 * 1. 验证传入参数数量是否与函数定义的参数数量匹配
 * 2. 创建新的执行环境，父环境设置为函数的闭包环境
 * 3. 将传入的参数值绑定到对应的参数名
 * 4. 切换解释器的当前环境到新创建的函数环境
 * 5. 重置全局返回状态
 * 6. 执行函数体语句
 * 7. 恢复解释器的原始环境
 * 8. 清理函数执行环境
 * 9. 返回函数的执行结果
 *
 * @note 此函数会修改全局的returnStatus状态
 * @note 函数执行过程中会临时切换解释器环境
 */
static Value callFunction(Interpreter *interpreter, Function *function,
						  Value *arguments, int argCount)
{
	if (function->arity != argCount)
	{
		runtimeError(interpreter, "期望 %d 个参数，但得到 %d 个。",
					 function->arity, argCount);
		return createNull();
	}

	// 创建一个新环境，父环境是函数的闭包环境
	Environment env;
	initEnvironment(&env, function->closure);

	// 将参数绑定到参数名
	for (int i = 0; i < argCount; i++)
	{
		defineVariable(&env, function->paramNames[i], arguments[i]);
	}

	// 保存当前环境
	Environment *previous = interpreter->environment;
	interpreter->environment = &env;

	// 重置返回状态
	returnStatus.hasReturn = false;
	returnStatus.value = createNull();

	// 执行函数体
	execute(interpreter, function->body);

	// 恢复环境
	interpreter->environment = previous;

	// 释放函数环境
	freeEnvironment(&env);

	// 返回函数的返回值
	if (returnStatus.hasReturn)
	{
		Value result = returnStatus.value;
		returnStatus.hasReturn = false;
		return result;
	}

	return createNull(); // 如果没有返回语句，返回null
}
// 检查解释器是否有错误
bool hadInterpreterError(Interpreter *interpreter)
{
	return interpreter != NULL && interpreter->hadError;
}

// 获取解释器错误消息
const char *getInterpreterError(Interpreter *interpreter)
{
	if (interpreter == NULL)
		return "解释器未初始化";
	return interpreter->errorMessage;
}

// 释放解释器资源
void freeInterpreter(Interpreter *interpreter)
{
	if (interpreter == NULL)
		return;

	// 释放全局环境
	if (interpreter->globals != NULL)
	{
		// 确保不会重复释放
		if (interpreter->environment == interpreter->globals)
		{
			interpreter->environment = NULL;
		}

		freeEnvironment(interpreter->globals);
		free(interpreter->globals);
		interpreter->globals = NULL;
	}

	// 重置当前环境指针（已被释放或指向其他环境）
	interpreter->environment = NULL;

	// 如果有main函数指针，只重置它，不尝试释放
	// 因为这个函数对象应该在全局环境的清理中已被释放
	interpreter->mainFunction = NULL;
	interpreter->hasMainFunction = false;

	// 重置错误状态
	interpreter->hadError = false;
	interpreter->errorMessage[0] = '\0';

	// 重置返回状态 (确保正确获取returnStatus)
	// returnStatus.hasReturn = false;
	// freeValue(returnStatus.value);
	// returnStatus.value = createNull();
}