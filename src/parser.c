#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

// 前向声明所有解析函数
static Expr *expression(Parser *parser);
static Expr *assignment(Parser *parser);
static Expr *equality(Parser *parser);
static Expr *comparison(Parser *parser);
static Expr *term(Parser *parser);
static Expr *factor(Parser *parser);
static Expr *unary(Parser *parser);
static Expr *call(Parser *parser);
static Expr *primary(Parser *parser);
static Expr *finishCall(Parser *parser, Expr *callee);
static Expr *logicalOr(Parser *parser);
static Expr *logicalAnd(Parser *parser);
static Expr *arrayLiteral(Parser *parser);

static Stmt *declaration(Parser *parser);
static Stmt *functionDeclaration(Parser *parser);
static Stmt *varDeclaration(Parser *parser);
static Stmt *constDeclaration(Parser *parser);
static Stmt *statement(Parser *parser);
static Stmt *expressionStatement(Parser *parser);
static Stmt *blockStatement(Parser *parser);
static Stmt *ifStatement(Parser *parser);
static Stmt *whileStatement(Parser *parser);
static Stmt *forStatement(Parser *parser);
static Stmt *returnStatement(Parser *parser);
static Stmt *switchStatement(Parser *parser);
static Stmt *breakStatement(Parser *parser);
static Stmt *doWhileStatement(Parser *parser);
static Stmt *enumDeclaration(Parser *parser);

// 辅助函数声明
static int match(Parser *parser, TokenType type);
static int check(Parser *parser, TokenType type);
static Token advance(Parser *parser);
static Token peek(Parser *parser);
static Token previous(Parser *parser);
static int isAtEnd(Parser *parser);
static Token consume(Parser *parser, TokenType type, const char *message);
static void synchronize(Parser *parser);
static void error(Parser *parser, const char *message);
static TypeAnnotation parseTypeAnnotation(Parser *parser);

// 初始化语法分析器
void initParser(Parser *parser, Token *tokens, int count)
{
    parser->tokens = tokens;
    parser->count = count;
    parser->current = 0;
    parser->hadError = 0;
    parser->errorMsg[0] = '\0';
}

// 解析整个程序，返回语句列表
Stmt **parse(Parser *parser, int *stmtCount)
{
    int capacity = 8;
    Stmt **statements = (Stmt **)malloc(capacity * sizeof(Stmt *));
    int count = 0;

    while (!isAtEnd(parser))
    {
        // 保存当前位置以检测进度
        int currentPos = parser->current;

        parser->hadError = 0; // 为每个顶级声明重置错误标志
        Stmt *stmt = declaration(parser);

        if (stmt != NULL)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                statements = (Stmt **)realloc(statements, capacity * sizeof(Stmt *));
            }
            statements[count++] = stmt;
        }

        if (parser->hadError)
        {
            synchronize(parser);

            // 如果 synchronize 没有前进，手动前进以避免无限循环
            if (currentPos == parser->current && !isAtEnd(parser))
            {
                advance(parser);
            }
        }
        else if (stmt == NULL && currentPos == parser->current && !isAtEnd(parser))
        {
            // 即使没有错误，如果declaration返回NULL且没有进度，也要前进
            error(parser, "Failed to parse declaration.");
            advance(parser);
        }
    }

    *stmtCount = count;
    return statements;
}

// 检查是否有语法错误
int hadParseError(Parser *parser)
{
    return parser->hadError;
}

// 获取错误信息
const char *getParseErrorMsg(Parser *parser)
{
    return parser->errorMsg;
}

/**
 * 解析声明语句
 *
 * 该函数负责解析各种类型的声明语句，包括：
 * - 静态声明 (static关键字)
 * - 函数声明 (function关键字)
 * - 变量声明 (var关键字)
 * - 常量声明 (const关键字)
 * - 枚举声明 (enum关键字)
 *
 * 对于变量声明，支持以下特性：
 * - 多变量声明：var a, b, c;
 * - 类型注解：var a: int;
 * - 初始化表达式：var a = 10;
 * - 组合使用：var a, b: int = 5;
 * - 静态声明：static var globalVar: int = 0;
 *
 * @param parser 解析器实例指针
 * @return 返回解析得到的声明语句指针，如果解析失败则返回NULL
 *
 * @note 函数会动态分配内存来存储多个变量名，并在发生错误时自动释放内存
 * @note 如果不匹配任何声明类型，则调用statement()函数解析普通语句
 */
static Stmt *declaration(Parser *parser)
{
    bool isStatic = false;

    // 检查是否有 static 关键字
    if (match(parser, TOKEN_STATIC))
    {
        isStatic = true;
    }

    if (match(parser, TOKEN_FUNCTION))
    {
        Stmt *funcStmt = functionDeclaration(parser);
        if (funcStmt != NULL && isStatic)
        {
            funcStmt->as.function.isStatic = true;
        }
        return funcStmt;
    }

    if (match(parser, TOKEN_ENUM))
    {
        if (isStatic)
        {
            error(parser, "Static enum declarations are not supported.");
            return NULL;
        }
        return enumDeclaration(parser);
    }

    if (match(parser, TOKEN_VAR))
    {
        // 创建存储多个变量名的数组
        int capacity = 4;
        int count = 0;
        Token *names = (Token *)malloc(sizeof(Token) * capacity);

        // 解析第一个变量名
        Token name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
        names[count++] = name;

        // 处理连续的变量声明（用逗号分隔）
        while (match(parser, TOKEN_COMMA))
        {
            if (count >= capacity)
            {
                capacity *= 2;
                names = (Token *)realloc(names, sizeof(Token) * capacity);
            }

            name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name after ','.");
            if (parser->hadError)
            {
                free(names);
                return NULL;
            }
            names[count++] = name;
        }

        // 处理类型注解
        TypeAnnotation typeAnnotation;
        typeAnnotation.kind = TYPE_SIMPLE;
        typeAnnotation.as.simple = TYPE_ANY; // 默认值

        if (match(parser, TOKEN_COLON))
        {
            // 使用 parseTypeAnnotation 来解析类型
            typeAnnotation = parseTypeAnnotation(parser);
            if (parser->hadError)
            {
                free(names);
                return NULL;
            }
        }

        // 处理初始值（可选，所有变量共用相同初始值）
        Expr *initializer = NULL;
        if (match(parser, TOKEN_ASSIGN))
        {
            initializer = expression(parser);
        }

        consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
        if (parser->hadError)
        {
            if (initializer)
                freeExpr(initializer);
            free(names);
            return NULL;
        }

        // 为多个变量创建声明语句
        if (count == 1)
        {
            // 只有一个变量，直接创建并返回单个语句
            Stmt *stmt = createVarStmt(names[0], typeAnnotation, initializer);
            if (stmt != NULL && isStatic)
            {
                stmt->as.var.isStatic = true;
            }
            free(names);
            return stmt;
        }
        else
        {
            if (isStatic)
            {
                error(parser, "Static multi-variable declarations are not currently supported.");
                if (initializer)
                    freeExpr(initializer);
                free(names);
                return NULL;
            }
            return createMultiVarStmt(names, count, typeAnnotation, initializer);
        }
    }

    if (match(parser, TOKEN_CONST)) // 添加常量声明解析
    {
        Stmt *constStmt = constDeclaration(parser);
        if (constStmt != NULL && isStatic)
        {
            constStmt->as.constStmt.isStatic = true;
        }
        return constStmt;
    }

    if (isStatic)
    {
        error(parser, "Expected declaration after 'static'.");
        return NULL;
    }

    return statement(parser);
}

/**
 * 解析函数声明语句
 *
 * 此函数负责解析函数声明的完整语法，包括：
 * - 函数名
 * - 参数列表（支持可变参数，第一个参数必须使用 var 关键字）
 * - 参数类型注解（可选，默认为 TYPE_ANY）
 * - 返回类型注解（可选，默认为 TYPE_VOID）
 * - 函数体
 *
 * 语法格式：
 * function_name(var param1: type1, param2: type2, ...): return_type {
 *     // 函数体
 * }
 *
 * 参数规则：
 * - 最多支持 255 个参数
 * - 第一个参数必须使用 var 关键字声明
 * - 后续参数默认继承第一个参数的 var 状态
 * - 参数类型注解为可选，使用冒号分隔
 *
 * @param parser 解析器实例指针
 * @return 成功时返回函数声明语句节点，失败时返回 NULL
 *
 * @note 函数会自动管理内存分配，在错误情况下会释放已分配的内存
 * @note 支持动态扩展参数数组容量，初始容量为 8，不足时翻倍扩展
 */
static Stmt *functionDeclaration(Parser *parser)
{
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    if (parser->hadError)
        return NULL;

    consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    if (parser->hadError)
        return NULL;

    // 参数列表
    Token *parameters = NULL;
    Token *paramTokenTypes = NULL;     // 重命名以区分
    TypeAnnotation *paramTypes = NULL; // 新增：用于存储转换后的类型注解
    bool *paramHasVarFlags = NULL;
    int paramCount = 0;
    int paramCapacity = 0;

    if (!check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (paramCount >= 255)
            {
                error(parser, "Cannot have more than 255 parameters.");
                if (parameters)
                    free(parameters);
                if (paramTokenTypes)
                    free(paramTokenTypes);
                if (paramTypes)
                    free(paramTypes);
                if (paramHasVarFlags)
                    free(paramHasVarFlags);
                return NULL;
            }

            bool hasVar = false;
            if (match(parser, TOKEN_VAR))
            {
                hasVar = true;
            }
            else if (paramCount == 0)
            {
                // 第一个参数必须有 var 关键字
                error(parser, "First function parameter must be declared with 'var' keyword.");
                if (parameters)
                    free(parameters);
                if (paramTokenTypes)
                    free(paramTokenTypes);
                if (paramTypes)
                    free(paramTypes);
                if (paramHasVarFlags)
                    free(paramHasVarFlags);
                return NULL;
            }
            else
            {
                // 后续参数默认继承第一个参数的 var 状态
                hasVar = true;
            }

            // 参数名
            Token param = consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            if (parser->hadError)
            {
                if (parameters)
                    free(parameters);
                if (paramTokenTypes)
                    free(paramTokenTypes);
                if (paramTypes)
                    free(paramTypes);
                if (paramHasVarFlags)
                    free(paramHasVarFlags);
                return NULL;
            }

            // 参数类型
            Token paramType = {0};
            paramType.type = TOKEN_VOID; // 默认类型
            TypeAnnotation paramTypeAnnotation;
            paramTypeAnnotation.kind = TYPE_SIMPLE;
            paramTypeAnnotation.as.simple = TYPE_ANY;

            if (match(parser, TOKEN_COLON))
            {
                // 解析参数类型
                paramTypeAnnotation = parseTypeAnnotation(parser);
                if (parser->hadError)
                {
                    if (parameters)
                        free(parameters);
                    if (paramTokenTypes)
                        free(paramTokenTypes);
                    if (paramTypes)
                        free(paramTypes);
                    if (paramHasVarFlags)
                        free(paramHasVarFlags);
                    return NULL;
                }
            }

            // 扩展数组容量
            if (paramCount >= paramCapacity)
            {
                paramCapacity = paramCapacity == 0 ? 8 : paramCapacity * 2;
                parameters = (Token *)realloc(parameters, paramCapacity * sizeof(Token));
                paramTokenTypes = (Token *)realloc(paramTokenTypes, paramCapacity * sizeof(Token));
                paramTypes = (TypeAnnotation *)realloc(paramTypes, paramCapacity * sizeof(TypeAnnotation));
                paramHasVarFlags = (bool *)realloc(paramHasVarFlags, paramCapacity * sizeof(bool));
            }

            // 添加参数信息
            parameters[paramCount] = param;
            paramTokenTypes[paramCount] = paramType;
            paramTypes[paramCount] = paramTypeAnnotation;
            paramHasVarFlags[paramCount] = hasVar;
            paramCount++;

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    if (parser->hadError)
    {
        if (parameters)
            free(parameters);
        if (paramTokenTypes)
            free(paramTokenTypes);
        if (paramTypes)
            free(paramTypes);
        if (paramHasVarFlags)
            free(paramHasVarFlags);
        return NULL;
    }

    // 检查是否有返回类型注解（冒号后跟类型）
    TypeAnnotation returnTypeAnnotation;
    returnTypeAnnotation.kind = TYPE_SIMPLE;
    returnTypeAnnotation.as.simple = TYPE_VOID; // 默认返回类型为 void

    if (match(parser, TOKEN_COLON))
    {
        // 解析返回类型
        returnTypeAnnotation = parseTypeAnnotation(parser);
        if (parser->hadError)
        {
            if (parameters)
                free(parameters);
            if (paramTokenTypes)
                free(paramTokenTypes);
            if (paramTypes)
                free(paramTypes);
            if (paramHasVarFlags)
                free(paramHasVarFlags);
            return NULL;
        }
    }

    // 函数体
    consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    if (parser->hadError)
    {
        if (parameters)
            free(parameters);
        if (paramTokenTypes)
            free(paramTokenTypes);
        if (paramTypes)
            free(paramTypes);
        if (paramHasVarFlags)
            free(paramHasVarFlags);
        return NULL;
    }

    Stmt *body = blockStatement(parser);
    if (parser->hadError)
    {
        if (parameters)
            free(parameters);
        if (paramTokenTypes)
            free(paramTokenTypes);
        if (paramTypes)
            free(paramTypes);
        if (paramHasVarFlags)
            free(paramHasVarFlags);
        return NULL;
    }

    // 现在传递正确的类型
    Stmt *result = createFunctionStmt(name, parameters, paramHasVarFlags, paramTypes, paramCount, returnTypeAnnotation, body);

    // 清理临时的Token类型数组
    if (paramTokenTypes)
        free(paramTokenTypes);

    return result;
}

// 解析变量声明
static Stmt *varDeclaration(Parser *parser)
{
    // 创建存储多个变量名的数组
    int capacity = 4;
    int count = 0;
    Token *names = (Token *)malloc(sizeof(Token) * capacity);

    // 解析第一个变量名
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    if (parser->hadError)
    {
        free(names);
        return NULL;
    }
    names[count++] = name;

    // 处理连续的变量声明（用逗号分隔）
    while (match(parser, TOKEN_COMMA))
    {
        if (count >= capacity)
        {
            capacity *= 2;
            names = (Token *)realloc(names, sizeof(Token) * capacity);
        }

        name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name after ','.");
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
        names[count++] = name;
    }

    // 处理类型注解
    TypeAnnotation typeAnnotation;
    typeAnnotation.kind = TYPE_SIMPLE;
    typeAnnotation.as.simple = TYPE_ANY; // 默认值

    if (match(parser, TOKEN_COLON))
    {
        // 解析类型
        typeAnnotation = parseTypeAnnotation(parser);
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
    }

    // 处理初始值（可选）
    Expr *initializer = NULL;
    if (match(parser, TOKEN_ASSIGN))
    {
        initializer = expression(parser);
        if (parser->hadError)
        {
            if (initializer)
                freeExpr(initializer);
            free(names);
            return NULL;
        }
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    if (parser->hadError)
    {
        if (initializer)
            freeExpr(initializer);
        free(names);
        return NULL;
    }

    // 为多个变量创建声明语句
    if (count == 1)
    {
        // 只有一个变量，直接创建并返回单个语句
        Stmt *stmt = createVarStmt(names[0], typeAnnotation, initializer);
        free(names);
        return stmt;
    }
    else
    {
        return createMultiVarStmt(names, count, typeAnnotation, initializer);
    }
}

static Stmt *constDeclaration(Parser *parser)
{
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect constant name.");
    if (parser->hadError)
        return NULL;

    TypeAnnotation typeAnnotation;
    typeAnnotation.kind = TYPE_SIMPLE;
    typeAnnotation.as.simple = TYPE_ANY; // 默认值

    // 处理类型注解
    if (match(parser, TOKEN_COLON))
    {
        typeAnnotation = parseTypeAnnotation(parser);
        if (parser->hadError)
            return NULL;
    }

    // 常量必须有初始值
    if (!match(parser, TOKEN_ASSIGN))
    {
        error(parser, "Constants must be initialized.");
        return NULL;
    }

    Expr *initializer = expression(parser);
    if (parser->hadError)
    {
        if (initializer)
            freeExpr(initializer);
        return NULL;
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after constant declaration.");
    if (parser->hadError)
    {
        if (initializer)
            freeExpr(initializer);
        return NULL;
    }

    return createConstStmt(name, typeAnnotation, initializer);
}

// 解析语句
static Stmt *statement(Parser *parser)
{
    if (match(parser, TOKEN_IF))
    {
        return ifStatement(parser);
    }

    if (match(parser, TOKEN_WHILE))
    {
        return whileStatement(parser);
    }

    if (match(parser, TOKEN_FOR))
    {
        return forStatement(parser);
    }

    if (match(parser, TOKEN_DO))
        return doWhileStatement(parser);

    if (match(parser, TOKEN_RETURN))
    {
        return returnStatement(parser);
    }

    if (match(parser, TOKEN_SWITCH))
    {
        return switchStatement(parser);
    }

    if (match(parser, TOKEN_BREAK))
    {
        return breakStatement(parser);
    }

    if (match(parser, TOKEN_LBRACE))
    {
        return blockStatement(parser);
    }

    return expressionStatement(parser);
}

// 解析表达式语句
static Stmt *expressionStatement(Parser *parser)
{

    Expr *expr = expression(parser);

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");

    if (parser->hadError)
    {
        freeExpr(expr);
        return NULL;
    }

    Stmt *stmt = createExpressionStmt(expr); // 先保存结果
    return stmt;                             // 添加这行以返回创建的语句
}

// 解析代码块
static Stmt *blockStatement(Parser *parser)
{
    int capacity = 8;
    Stmt **statements = (Stmt **)malloc(capacity * sizeof(Stmt *));
    int count = 0;

    while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        // 保存当前位置以检测进度
        int currentPos = parser->current;

        Stmt *stmt = declaration(parser);

        if (stmt != NULL)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                statements = (Stmt **)realloc(statements, capacity * sizeof(Stmt *));
            }
            statements[count++] = stmt;
        }

        // 检查是否有进度，防止无限循环
        if (parser->hadError)
        {
            // 如果有错误，尝试恢复到下一个语句
            synchronize(parser);

            // 如果没有进度且未到达文件末尾，强制前进
            if (currentPos == parser->current && !isAtEnd(parser))
            {
                advance(parser);
            }
        }
        else if (currentPos == parser->current && !isAtEnd(parser))
        {
            // 即使没有错误，如果没有进度也要防止无限循环
            error(parser, "Unexpected token in block statement.");
            advance(parser);
        }
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    if (parser->hadError)
    {
        for (int i = 0; i < count; i++)
        {
            freeStmt(statements[i]);
        }
        free(statements);
        return NULL;
    }

    return createBlockStmt(statements, count);
}

// 解析if语句
static Stmt *ifStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    if (parser->hadError)
        return NULL;

    Expr *condition = expression(parser);

    consume(parser, TOKEN_RPAREN, "Expect ')' after if condition.");
    if (parser->hadError)
    {
        freeExpr(condition);
        return NULL;
    }

    Stmt *thenBranch = statement(parser);
    Stmt *elseBranch = NULL;

    if (match(parser, TOKEN_ELSE))
    {
        elseBranch = statement(parser);
    }

    return createIfStmt(condition, thenBranch, elseBranch);
}

// 解析while循环
static Stmt *whileStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    if (parser->hadError)
        return NULL;

    Expr *condition = expression(parser);

    consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    if (parser->hadError)
    {
        freeExpr(condition);
        return NULL;
    }

    Stmt *body = statement(parser);

    return createWhileStmt(condition, body);
}

// 解析for循环
static Stmt *forStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'for'.");
    if (parser->hadError)
        return NULL;

    // 初始化部分
    Stmt *initializer = NULL;
    if (match(parser, TOKEN_SEMICOLON))
    {
        // 没有初始化
    }
    else if (check(parser, TOKEN_VAR))
    {
        advance(parser);
        initializer = varDeclaration(parser);
    }
    else if (check(parser, TOKEN_INT) || check(parser, TOKEN_FLOAT_TYPE) ||
             check(parser, TOKEN_STRING_TYPE) || check(parser, TOKEN_BOOL))
    {
        initializer = varDeclaration(parser);
    }
    else
    {
        initializer = expressionStatement(parser);
    }

    if (parser->hadError)
        return NULL;

    // 条件部分
    Expr *condition = NULL;
    if (!check(parser, TOKEN_SEMICOLON))
    {
        condition = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    if (parser->hadError)
    {
        if (initializer)
            freeStmt(initializer);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    // 增量部分
    Expr *increment = NULL;
    if (!check(parser, TOKEN_RPAREN))
    {
        increment = expression(parser);
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after for clauses.");
    if (parser->hadError)
    {
        if (initializer)
            freeStmt(initializer);
        if (condition)
            freeExpr(condition);
        if (increment)
            freeExpr(increment);
        return NULL;
    }

    // 循环体
    Stmt *body = statement(parser);

    return createForStmt(initializer, condition, increment, body);
}

// 解析return语句
static Stmt *returnStatement(Parser *parser)
{
    Token keyword = previous(parser);
    Expr *value = NULL;

    if (!check(parser, TOKEN_SEMICOLON))
    {
        value = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    if (parser->hadError)
    {
        if (value)
            freeExpr(value);
        return NULL;
    }

    return createReturnStmt(keyword, value);
}

// 解析 switch 语句
static Stmt *switchStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'switch'.");
    if (parser->hadError)
        return NULL;

    Expr *discriminant = expression(parser);
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after switch expression.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    consume(parser, TOKEN_LBRACE, "Expect '{' before switch body.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        return NULL;
    }

    // 解析 case 语句
    int capacity = 8;
    CaseStmt *cases = (CaseStmt *)malloc(capacity * sizeof(CaseStmt));
    int caseCount = 0;

    while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        if (caseCount >= capacity)
        {
            capacity *= 2;
            cases = (CaseStmt *)realloc(cases, capacity * sizeof(CaseStmt));
        }

        if (match(parser, TOKEN_CASE))
        {
            Expr *caseValue = expression(parser);
            if (parser->hadError)
            {
                freeExpr(discriminant);
                free(cases);
                return NULL;
            }

            consume(parser, TOKEN_COLON, "Expect ':' after case value.");
            if (parser->hadError)
            {
                freeExpr(discriminant);
                freeExpr(caseValue);
                free(cases);
                return NULL;
            }

            Stmt **caseStatements = NULL;
            int stmtCapacity = 4;
            int stmtCount = 0;
            caseStatements = (Stmt **)malloc(stmtCapacity * sizeof(Stmt *));

            // 解析直到遇到 case、default 或 }
            while (!check(parser, TOKEN_CASE) && !check(parser, TOKEN_DEFAULT) &&
                   !check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
            {
                if (stmtCount >= stmtCapacity)
                {
                    stmtCapacity *= 2;
                    caseStatements = (Stmt **)realloc(caseStatements, stmtCapacity * sizeof(Stmt *));
                }

                Stmt *stmt = statement(parser);
                if (parser->hadError)
                {
                    // 清理内存
                    for (int i = 0; i < stmtCount; i++)
                    {
                        freeStmt(caseStatements[i]);
                    }
                    free(caseStatements);
                    freeExpr(discriminant);
                    freeExpr(caseValue);
                    free(cases);
                    return NULL;
                }
                caseStatements[stmtCount++] = stmt;
            }

            // 创建块语句
            Stmt *caseBody = createBlockStmt(caseStatements, stmtCount);

            cases[caseCount].value = caseValue;
            cases[caseCount].body = caseBody;
            caseCount++;
        }
        else if (match(parser, TOKEN_DEFAULT))
        {
            consume(parser, TOKEN_COLON, "Expect ':' after 'default'.");
            if (parser->hadError)
            {
                freeExpr(discriminant);
                free(cases);
                return NULL;
            }

            // 类似地处理 default 语句体
            Stmt **defaultStatements = NULL;
            int stmtCapacity = 4;
            int stmtCount = 0;
            defaultStatements = (Stmt **)malloc(stmtCapacity * sizeof(Stmt *));

            while (!check(parser, TOKEN_CASE) && !check(parser, TOKEN_DEFAULT) &&
                   !check(parser, TOKEN_RBRACE) && !isAtEnd(parser))
            {
                if (stmtCount >= stmtCapacity)
                {
                    stmtCapacity *= 2;
                    defaultStatements = (Stmt **)realloc(defaultStatements, stmtCapacity * sizeof(Stmt *));
                }

                Stmt *stmt = statement(parser);
                if (parser->hadError)
                {
                    for (int i = 0; i < stmtCount; i++)
                    {
                        freeStmt(defaultStatements[i]);
                    }
                    free(defaultStatements);
                    freeExpr(discriminant);
                    free(cases);
                    return NULL;
                }
                defaultStatements[stmtCount++] = stmt;
            }

            Stmt *defaultBody = createBlockStmt(defaultStatements, stmtCount);

            cases[caseCount].value = NULL; // NULL 表示 default
            cases[caseCount].body = defaultBody;
            caseCount++;
        }
        else
        {
            error(parser, "Expect 'case' or 'default' in switch statement.");
            freeExpr(discriminant);
            free(cases);
            return NULL;
        }
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after switch body.");
    if (parser->hadError)
    {
        freeExpr(discriminant);
        free(cases);
        return NULL;
    }

    return createSwitchStmt(discriminant, cases, caseCount);
}

// 解析 break 语句
static Stmt *breakStatement(Parser *parser)
{
    Token keyword = previous(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'break'.");
    if (parser->hadError)
        return NULL;

    return createBreakStmt(keyword);
}

static Stmt *doWhileStatement(Parser *parser)
{

    // 解析循环体
    Stmt *body = statement(parser);
    if (parser->hadError)
    {
        return NULL;
    }

    // 期望 while 关键字
    consume(parser, TOKEN_WHILE, "Expect 'while' after do body.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 期望左括号
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 调试：检查当前 token
    if (isAtEnd(parser))
    {
        error(parser, "Unexpected end of file in do-while condition.");
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 解析条件表达式
    Expr *condition = expression(parser);
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        return NULL;
    }

    if (condition == NULL)
    {
        error(parser, "Failed to parse do-while condition expression.");
        if (body)
            freeStmt(body);
        return NULL;
    }

    // 期望右括号
    consume(parser, TOKEN_RPAREN, "Expect ')' after do-while condition.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    // 期望分号
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after do-while statement.");
    if (parser->hadError)
    {
        if (body)
            freeStmt(body);
        if (condition)
            freeExpr(condition);
        return NULL;
    }

    return createDoWhileStmt(body, condition);
}

static Stmt *enumDeclaration(Parser *parser)
{

    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect enum name.");
    if (parser->hadError)
        return NULL;

    consume(parser, TOKEN_LBRACE, "Expect '{' before enum body.");
    if (parser->hadError)
        return NULL;

    // 解析枚举成员
    int capacity = 8;
    EnumMember *members = (EnumMember *)malloc(capacity * sizeof(EnumMember));
    int memberCount = 0;
    int currentValue = 0;

    if (!check(parser, TOKEN_RBRACE))
    {
        do
        {
            if (memberCount >= capacity)
            {
                capacity *= 2;
                members = (EnumMember *)realloc(members, capacity * sizeof(EnumMember));
            }

            Token memberName = consume(parser, TOKEN_IDENTIFIER, "Expect enum member name.");
            if (parser->hadError)
            {
                free(members);
                return NULL;
            }

            Expr *value = NULL;
            if (match(parser, TOKEN_ASSIGN))
            {
                value = expression(parser);
                if (parser->hadError)
                {
                    free(members);
                    return NULL;
                }

                // 如果是数字字面量，更新当前值
                if (value->type == EXPR_LITERAL && value->as.literal.value.type == TOKEN_INTEGER)
                {
                    currentValue = value->as.literal.value.value.intValue;
                }
            }

            members[memberCount].name = memberName;
            members[memberCount].value = value;
            memberCount++;

            if (value == NULL)
            {
                currentValue++; // 如果没有显式值，自动递增
            }
            else
            {
                currentValue++; // 为下一个成员准备
            }

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after enum body.");
    if (parser->hadError)
    {
        free(members);
        return NULL;
    }

    Stmt *stmt = createEnumStmt(name, members, memberCount);

    return stmt;
}

// 解析表达式
static Expr *expression(Parser *parser)
{
    return assignment(parser);
}

// 解析赋值表达式
static Expr *assignment(Parser *parser)
{
    Expr *expr = logicalOr(parser);

    if (expr == NULL)
    {
        return NULL; // 直接返回NULL，不设置错误消息
    }

    if (match(parser, TOKEN_ASSIGN))
    {
        Expr *value = assignment(parser);
        if (value == NULL)
        {
            freeExpr(expr);
            return NULL;
        }

        if (expr->type == EXPR_VARIABLE)
        {
            Token name = expr->as.variable.name;
            freeExpr(expr);
            return createAssignExpr(name, value);
        }
        else if (expr->type == EXPR_ARRAY_ACCESS)
        {
            // 数组元素赋值
            return createArrayAssignExpr(expr->as.arrayAccess.array,
                                         expr->as.arrayAccess.index, value);
        }

        error(parser, "Invalid assignment target.");
        freeExpr(expr);
        freeExpr(value);
        return NULL;
    }

    return expr;
}

// 解析相等性表达式
static Expr *equality(Parser *parser)
{
    Expr *expr = comparison(parser);

    while (match(parser, TOKEN_EQ) || match(parser, TOKEN_NE))
    {
        TokenType operator = previous(parser).type;
        Expr *right = comparison(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析比较表达式
static Expr *comparison(Parser *parser)
{
    Expr *expr = term(parser);

    while (match(parser, TOKEN_LT) || match(parser, TOKEN_LE) ||
           match(parser, TOKEN_GT) || match(parser, TOKEN_GE) ||
           match(parser, TOKEN_IN))
    {
        TokenType operator = previous(parser).type;
        Expr *right = term(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析加减表达式
static Expr *term(Parser *parser)
{
    Expr *expr = factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = factor(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析乘除表达式
static Expr *factor(Parser *parser)
{
    Expr *expr = unary(parser);

    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE) || match(parser, TOKEN_MODULO))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析一元表达式
static Expr *unary(Parser *parser)
{
    // 检查类型转换：(type)expression
    if (match(parser, TOKEN_LPAREN))
    {
        // 检查是否为类型转换
        BaseType castType = TYPE_ANY;
        bool isCast = false;

        if (check(parser, TOKEN_INT))
        {
            castType = TYPE_INT;
            isCast = true;
        }
        else if (check(parser, TOKEN_FLOAT_TYPE))
        {
            castType = TYPE_FLOAT;
            isCast = true;
        }
        else if (check(parser, TOKEN_DOUBLE))
        {
            castType = TYPE_DOUBLE;
            isCast = true;
        }
        else if (check(parser, TOKEN_STRING_TYPE))
        {
            castType = TYPE_STRING;
            isCast = true;
        }
        else if (check(parser, TOKEN_BOOL))
        {
            castType = TYPE_BOOL;
            isCast = true;
        }

        if (isCast)
        {
            advance(parser); // 消费类型 token
            consume(parser, TOKEN_RPAREN, "Expect ')' after cast type.");
            if (parser->hadError)
                return NULL;

            Expr *expression = unary(parser);
            if (parser->hadError)
            {
                if (expression)
                    freeExpr(expression);
                return NULL;
            }

            return createCastExpr(castType, expression);
        }
        else
        {
            // 不是类型转换，退回并按分组表达式处理
            parser->current--; // 退回 '('

            // 继续处理其他一元表达式
            if (match(parser, TOKEN_NOT) || match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS))
            {
                TokenType operator = previous(parser).type;
                Expr *right = unary(parser);
                if (parser->hadError)
                {
                    if (right)
                        freeExpr(right);
                    return NULL;
                }
                return createUnaryExpr(operator, right);
            }
        }
    }

    if (match(parser, TOKEN_NOT) || match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        if (parser->hadError)
        {
            if (right)
                freeExpr(right);
            return NULL;
        }
        return createUnaryExpr(operator, right);
    }

    // 处理前缀运算符
    if (match(parser, TOKEN_PLUS_PLUS) || match(parser, TOKEN_MINUS_MINUS))
    {
        TokenType operator = previous(parser).type;
        Expr *right = unary(parser);
        if (parser->hadError)
        {
            if (right)
                freeExpr(right);
            return NULL;
        }

        // 检查右操作数是否是变量
        if (right == NULL || right->type != EXPR_VARIABLE)
        {
            error(parser, "Prefix operators can only be applied to variables.");
            if (right)
                freeExpr(right);
            return NULL;
        }

        return createPrefixExpr(right, operator);
    }

    return call(parser);
}

// 解析调用表达式
static Expr *call(Parser *parser)
{
    Expr *expr = primary(parser);

    while (true)
    {
        if (match(parser, TOKEN_LPAREN))
        {
            expr = finishCall(parser, expr);
        }
        else if (match(parser, TOKEN_LBRACKET))
        {
            // 数组索引访问
            Expr *index = expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expect ']' after array index.");
            if (parser->hadError)
            {
                freeExpr(expr);
                freeExpr(index);
                return NULL;
            }
            expr = createArrayAccessExpr(expr, index);
        }
        else if (match(parser, TOKEN_PLUS_PLUS) || match(parser, TOKEN_MINUS_MINUS))
        {
            TokenType op = previous(parser).type;

            // 检查左操作数是否是变量
            if (expr->type != EXPR_VARIABLE)
            {
                error(parser, "Invalid left-hand side in postfix expression.");
                freeExpr(expr);
                return NULL;
            }

            expr = createPostfixExpr(expr, op);
            // 检查 createPostfixExpr 是否返回 NULL
            if (expr == NULL)
            {
                return NULL;
            }
        }
        else
        {
            break;
        }
    }

    return expr;
}

// 完成函数调用的解析
static Expr *finishCall(Parser *parser, Expr *callee)
{
    // 解析参数列表
    Expr **arguments = NULL;
    int argCount = 0;
    int capacity = 0;

    if (!check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (argCount >= 255)
            {
                error(parser, "Cannot have more than 255 arguments.");
                break;
            }

            Expr *arg = expression(parser);

            if (argCount >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                arguments = (Expr **)realloc(arguments, capacity * sizeof(Expr *));
            }

            arguments[argCount++] = arg;

        } while (match(parser, TOKEN_COMMA));
    }

    Token paren = consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    if (parser->hadError)
    {
        for (int i = 0; i < argCount; i++)
        {
            freeExpr(arguments[i]);
        }
        free(arguments);
        freeExpr(callee);
        return NULL;
    }

    return createCallExpr(callee, paren, arguments, argCount);
}

Expr *logicalOr(Parser *parser)
{
    Expr *expr = logicalAnd(parser);

    while (match(parser, TOKEN_OR))
    {
        TokenType operator = previous(parser).type;
        Expr *right = logicalAnd(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

Expr *logicalAnd(Parser *parser)
{
    Expr *expr = equality(parser);

    while (match(parser, TOKEN_AND))
    {
        TokenType operator = previous(parser).type;
        Expr *right = equality(parser);
        expr = createBinaryExpr(expr, operator, right);
    }

    return expr;
}

// 解析数组字面量
static Expr *arrayLiteral(Parser *parser)
{
    Expr **elements = NULL;
    int count = 0;
    int capacity = 0;

    if (!check(parser, TOKEN_RBRACKET))
    {
        do
        {
            if (count >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                elements = (Expr **)realloc(elements, capacity * sizeof(Expr *));
            }

            elements[count++] = expression(parser);

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RBRACKET, "Expect ']' after array elements.");
    if (parser->hadError)
    {
        for (int i = 0; i < count; i++)
        {
            freeExpr(elements[i]);
        }
        free(elements);
        return NULL;
    }

    return createArrayLiteralExpr(elements, count);
}

// 解析基本表达式
static Expr *primary(Parser *parser)
{
    if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_STRING))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        return createVariableExpr(previous(parser));
    }

    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_NULL))
    {
        return createLiteralExpr(previous(parser));
    }

    if (match(parser, TOKEN_LBRACKET))
    {
        return arrayLiteral(parser);
    }

    if (match(parser, TOKEN_LPAREN))
    {
        Expr *expr = expression(parser);
        consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        if (parser->hadError)
        {
            freeExpr(expr);
            return NULL;
        }
        return createGroupingExpr(expr);
    }

    if (parser->hadError)
    {
        return NULL;
    }

    error(parser, "Expect expression.");
    return NULL;
}

// 检查当前标记是否匹配指定类型
static int match(Parser *parser, TokenType type)
{
    if (check(parser, type))
    {
        advance(parser);
        return 1;
    }
    return 0;
}

// 检查当前标记是否是指定类型
static int check(Parser *parser, TokenType type)
{
    if (isAtEnd(parser))
        return 0;
    return peek(parser).type == type;
}

// 前进到下一个标记并返回它
static Token advance(Parser *parser)
{
    if (!isAtEnd(parser))
        parser->current++;
    return previous(parser);
}

// 获取当前标记
static Token peek(Parser *parser)
{
    return parser->tokens[parser->current];
}

// 获取上一个标记
static Token previous(Parser *parser)
{
    return parser->tokens[parser->current - 1];
}

// 检查是否到达标记流末尾
static int isAtEnd(Parser *parser)
{
    return peek(parser).type == TOKEN_EOF;
}

// 消费当前标记，如果类型匹配则前进，否则报错
static Token consume(Parser *parser, TokenType type, const char *message)
{
    if (check(parser, type))
    {
        return advance(parser);
    }

    error(parser, message);
    Token errorToken = {0};
    return errorToken;
}

// 错误处理：同步到下一个安全点
static void synchronize(Parser *parser)
{
    parser->hadError = 0; // 重置错误标志，尝试继续解析

    // 如果已经在文件末尾，直接返回
    if (isAtEnd(parser))
        return;

    advance(parser);

    while (!isAtEnd(parser))
    {
        if (previous(parser).type == TOKEN_SEMICOLON)
            return;

        switch (peek(parser).type)
        {
        case TOKEN_FUNCTION:
        case TOKEN_VAR:
        case TOKEN_CONST:
        case TOKEN_INT:
        case TOKEN_FLOAT_TYPE:
        case TOKEN_STRING_TYPE:
        case TOKEN_BOOL:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_FOR:
        case TOKEN_RETURN:
        case TOKEN_LBRACE:
        case TOKEN_RBRACE:
        case TOKEN_RBRACKET:
            return;
        default:
            break;
        }

        advance(parser);
    }
}

// 记录错误
static void error(Parser *parser, const char *message)
{
    parser->hadError = 1;
    snprintf(parser->errorMsg, sizeof(parser->errorMsg), "Line %d: Error: %s",
             peek(parser).line, message);
}

static TypeAnnotation parseTypeAnnotation(Parser *parser)
{
    TypeAnnotation type;

    // 解析基本类型
    BaseType baseType = TYPE_ANY;

    if (match(parser, TOKEN_INT))
    {
        baseType = TYPE_INT;
    }
    else if (match(parser, TOKEN_FLOAT_TYPE))
    {
        baseType = TYPE_FLOAT;
    }

    else if (match(parser, TOKEN_STRING_TYPE))
    {
        baseType = TYPE_STRING;
    }
    else if (match(parser, TOKEN_BOOL))
    {
        baseType = TYPE_BOOL;
    }
    else if (match(parser, TOKEN_VOID))
    {
        baseType = TYPE_VOID;
    }
    else
    {
        error(parser, "Expected type annotation.");
        type.kind = TYPE_SIMPLE;
        type.as.simple = TYPE_ANY;
        return type;
    }

    // 检查是否是数组类型
    if (match(parser, TOKEN_LBRACKET))
    {
        if (!match(parser, TOKEN_RBRACKET))
        {
            error(parser, "Expected ']' after '['.");
        }

        type.kind = TYPE_ARRAY;
        type.as.array.elementType = baseType;
        type.as.array.size = NULL; // 动态数组
    }
    else
    {
        type.kind = TYPE_SIMPLE;
        type.as.simple = baseType;
    }

    return type;
}