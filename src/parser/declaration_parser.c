#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/parser/declaration_parser.h"
#include "../include/parser/statement_parser.h"
#include "../include/parser/expression_parser.h"
#include "../include/parser/type_parser.h"

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
Stmt *declaration(Parser *parser)
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

    if (match(parser, TOKEN_STRUCT))
    {
        if (isStatic)
        {
            error(parser, "Static struct declarations are not supported.");
            return NULL;
        }
        return structDeclaration(parser);
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
            // 多个变量声明
            Stmt *stmt = createMultiVarStmt(names, count, typeAnnotation, initializer);
            if (stmt != NULL && isStatic)
            {
                // 为多变量声明添加静态标记支持
                stmt->as.multiVar.isStatic = true;
            }
            return stmt;
        }
    }

    if (match(parser, TOKEN_CONST)) // 添加常量声明解析
    {
        Stmt *constStmt = constDeclaration(parser);
        if (constStmt != NULL && isStatic)
        {
            // 根据语句类型设置静态标记
            if (constStmt->type == STMT_CONST)
            {
                constStmt->as.constStmt.isStatic = true;
            }
            else if (constStmt->type == STMT_MULTI_CONST)
            {
                constStmt->as.multiConst.isStatic = true;
            }
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
Stmt *functionDeclaration(Parser *parser)
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
Stmt *varDeclaration(Parser *parser)
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

Stmt *constDeclaration(Parser *parser)
{
    // 创建存储多个常量名的数组
    int capacity = 4;
    int count = 0;
    Token *names = (Token *)malloc(sizeof(Token) * capacity);

    // 解析第一个常量名
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect constant name.");
    if (parser->hadError)
    {
        free(names);
        return NULL;
    }
    names[count++] = name;

    // 处理连续的常量声明（用逗号分隔）
    while (match(parser, TOKEN_COMMA))
    {
        if (count >= capacity)
        {
            capacity *= 2;
            names = (Token *)realloc(names, sizeof(Token) * capacity);
        }

        name = consume(parser, TOKEN_IDENTIFIER, "Expect constant name after ','.");
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
        names[count++] = name;
    }

    TypeAnnotation typeAnnotation;
    typeAnnotation.kind = TYPE_SIMPLE;
    typeAnnotation.as.simple = TYPE_ANY; // 默认值

    // 处理类型注解
    if (match(parser, TOKEN_COLON))
    {
        typeAnnotation = parseTypeAnnotation(parser);
        if (parser->hadError)
        {
            free(names);
            return NULL;
        }
    }

    // 常量必须有初始值
    if (!match(parser, TOKEN_ASSIGN))
    {
        error(parser, "Constants must be initialized.");
        free(names);
        return NULL;
    }

    // 解析初始值列表
    int initializerCapacity = 4;
    int initializerCount = 0;
    Expr **initializers = (Expr **)malloc(sizeof(Expr *) * initializerCapacity);
    
    // 解析第一个初始值
    Expr *firstInitializer = expression(parser);
    if (parser->hadError)
    {
        if (firstInitializer)
            freeExpr(firstInitializer);
        free(names);
        free(initializers);
        return NULL;
    }
    initializers[initializerCount++] = firstInitializer;

    // 处理多个初始值（用逗号分隔）
    while (match(parser, TOKEN_COMMA) && initializerCount < count)
    {
        if (initializerCount >= initializerCapacity)
        {
            initializerCapacity *= 2;
            initializers = (Expr **)realloc(initializers, sizeof(Expr *) * initializerCapacity);
        }

        Expr *nextInitializer = expression(parser);
        if (parser->hadError)
        {
            for (int i = 0; i < initializerCount; i++)
            {
                freeExpr(initializers[i]);
            }
            if (nextInitializer)
                freeExpr(nextInitializer);
            free(names);
            free(initializers);
            return NULL;
        }
        initializers[initializerCount++] = nextInitializer;
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after constant declaration.");
    if (parser->hadError)
    {
        for (int i = 0; i < initializerCount; i++)
        {
            freeExpr(initializers[i]);
        }
        free(names);
        free(initializers);
        return NULL;
    }

    // 为多个常量创建声明语句
    if (count == 1)
    {
        // 只有一个常量，直接创建并返回单个语句
        Stmt *stmt = createConstStmt(names[0], typeAnnotation, initializers[0]);
        free(names);
        free(initializers);
        return stmt;
    }
    else
    {
        // 检查初始值数量
        if (initializerCount == 1)
        {
            // 只有一个初始值，直接使用，不复制指针
            return createMultiConstStmt(names, count, typeAnnotation, initializers, initializerCount);
        }
        else if (initializerCount == count)
        {
            // 每个常量有自己的初始值
            return createMultiConstStmt(names, count, typeAnnotation, initializers, initializerCount);
        }
        else
        {
            error(parser, "Number of initializers must be 1 or equal to number of constants.");
            for (int i = 0; i < initializerCount; i++)
            {
                freeExpr(initializers[i]);
            }
            free(names);
            free(initializers);
            return NULL;
        }
    }
}

Stmt *enumDeclaration(Parser *parser)
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

Stmt *structDeclaration(Parser *parser)
{
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect struct name.");
    if (parser->hadError)
        return NULL;

    consume(parser, TOKEN_LBRACE, "Expect '{' before struct body.");
    if (parser->hadError)
        return NULL;

    // 解析结构体字段
    int capacity = 8;
    StructField *fields = (StructField *)malloc(capacity * sizeof(StructField));
    int fieldCount = 0;

    if (!check(parser, TOKEN_RBRACE))
    {
        do
        {
            if (fieldCount >= capacity)
            {
                capacity *= 2;
                fields = (StructField *)realloc(fields, capacity * sizeof(StructField));
            }

            Token fieldName = consume(parser, TOKEN_IDENTIFIER, "Expect field name.");
            if (parser->hadError)
            {
                free(fields);
                return NULL;
            }

            consume(parser, TOKEN_COLON, "Expect ':' after field name.");
            if (parser->hadError)
            {
                free(fields);
                return NULL;
            }

            TypeAnnotation fieldType = parseTypeAnnotation(parser);
            if (parser->hadError)
            {
                free(fields);
                return NULL;
            }

            fields[fieldCount].name = fieldName;
            fields[fieldCount].type = fieldType;
            fieldCount++;

            consume(parser, TOKEN_SEMICOLON, "Expect ';' after field declaration.");
            if (parser->hadError)
            {
                free(fields);
                return NULL;
            }

        } while (!check(parser, TOKEN_RBRACE) && !isAtEnd(parser));
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after struct body.");
    if (parser->hadError)
    {
        free(fields);
        return NULL;
    }

    Stmt *stmt = createStructStmt(name, fields, fieldCount);

    return stmt;
}
