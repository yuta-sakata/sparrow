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

static void executeExpression(Interpreter *interpreter, Stmt *stmt);
static void executeVar(Interpreter *interpreter, Stmt *stmt);
static void executeMultiVar(Interpreter *interpreter, Stmt *stmt);
static void executeBlock(Interpreter *interpreter, Stmt **statements, int count, Environment *environment);
static void executeIf(Interpreter *interpreter, Stmt *stmt);
static void executeWhile(Interpreter *interpreter, Stmt *stmt);
static void executeFor(Interpreter *interpreter, Stmt *stmt);
static void executeFunction(Interpreter *interpreter, Stmt *stmt);
static void executeReturn(Interpreter *interpreter, Stmt *stmt);

static Value callFunction(Interpreter *interpreter, Function *function, Value *arguments, int argCount);
static void runtimeError(Interpreter *interpreter, const char *format, ...);

typedef struct
{
	bool hasReturn;
	Value value;
} ReturnStatus;

// 静态变量
static ReturnStatus returnStatus;

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
	}

	// 如果不是已知的一元运算符
	freeValue(right);
	return createNull();
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
	case TOKEN_EQ: // 相等运算符 (==)
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
		char *str = malloc(len - 1);
		if (str == NULL)
		{
			return createNull();
		}
		strncpy(str, token.lexeme + 1, len - 2);
		str[len - 2] = '\0';

		Value val = createString(str);
		free(str);
		return val;
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

    // 添加调试输出
    printf("DEBUG: Looking for variable '%s'\n", expr->as.variable.name.lexeme);

    Value result = getVariable(interpreter->environment, expr->as.variable.name);
    
    // 检查是否找到变量
    if (result.type == VAL_NULL) {
        printf("DEBUG: Variable '%s' not found\n", expr->as.variable.name.lexeme);
    } else {
        printf("DEBUG: Variable '%s' found, type: %d\n", expr->as.variable.name.lexeme, result.type);
    }

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

// 执行函数声明
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

    // 添加调试输出
    printf("DEBUG: Defining function '%s'\n", function->name);

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
    
    // 添加调试输出确认函数已定义
    printf("DEBUG: Function '%s' defined in environment\n", function->name);
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

// 注册原生函数
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