#include "ast.h"

// 创建二元表达式
Expr *createBinaryExpr(Expr *left, TokenType op, Expr *right)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.op = op;
    expr->as.binary.right = right;
    return expr;
}

// 创建一元表达式
Expr *createUnaryExpr(TokenType op, Expr *right)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.op = op;
    expr->as.unary.right = right;
    return expr;
}

// 创建字面量表达式
Expr *createLiteralExpr(Token value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    return expr;
}

// 创建分组表达式
Expr *createGroupingExpr(Expr *expression)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;
    return expr;
}

// 创建变量引用
Expr *createVariableExpr(Token name)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;
    return expr;
}

// 创建赋值表达式
Expr *createAssignExpr(Token name, Expr *value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;
    return expr;
}

// 创建函数调用
Expr *createCallExpr(Expr *callee, Token paren, Expr **arguments, int argCount)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->as.call.callee = callee;
    expr->as.call.paren = paren;
    expr->as.call.arguments = arguments;
    expr->as.call.argCount = argCount;
    return expr;
}

Expr *createPostfixExpr(Expr *operand, TokenType op)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    expr->type = EXPR_POSTFIX;
    expr->as.postfix.operand = operand;
    expr->as.postfix.op = op;
    return expr;
}

Expr *createPrefixExpr(Expr *operand, TokenType op)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
    {
        return NULL;
    }
    expr->type = EXPR_PREFIX;
    expr->as.prefix.operand = operand;
    expr->as.prefix.op = op;
    return expr;
}

// 创建多变量声明
Stmt *createMultiVarStmt(Token *names, int count, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_MULTI_VAR;

    stmt->as.multiVar.names = names; // 直接使用传入的名称数组
    stmt->as.multiVar.count = count;
    stmt->as.multiVar.type = type;
    stmt->as.multiVar.initializer = initializer;
    stmt->as.multiVar.isStatic = false; // 默认非静态

    return stmt;
}

Stmt *createMultiConstStmt(Token *names, int count, TypeAnnotation type, Expr **initializers, int initializerCount)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_MULTI_CONST;
    stmt->as.multiConst.names = names;
    stmt->as.multiConst.count = count;
    stmt->as.multiConst.type = type;
    stmt->as.multiConst.initializers = initializers;
    stmt->as.multiConst.initializerCount = initializerCount;
    stmt->as.multiConst.isStatic = false; // 默认非静态
    return stmt;
}

// 创建表达式语句
Stmt *createExpressionStmt(Expr *expression)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        // 处理内存分配失败
        return NULL;
    }
    stmt->type = STMT_EXPRESSION;
    stmt->as.expression.expression = expression;
    return stmt;
}

// 创建变量声明
Stmt *createVarStmt(Token name, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;
    stmt->as.var.type = type;
    stmt->as.var.initializer = initializer;
    stmt->as.var.isStatic = false; // 默认非 static
    return stmt;
}

// 创建 static 变量声明
Stmt *createStaticVarStmt(Token name, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;
    stmt->as.var.type = type;
    stmt->as.var.initializer = initializer;
    stmt->as.var.isStatic = true;
    return stmt;
}

// 创建常量声明
Stmt *createConstStmt(Token name, TypeAnnotation type, Expr *initializer)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_CONST;
    stmt->as.constStmt.name = name;
    stmt->as.constStmt.type = type;
    stmt->as.constStmt.initializer = initializer;
    stmt->as.constStmt.isStatic = false; // 默认非 static

    return stmt;
}

// 创建代码块
Stmt *createBlockStmt(Stmt **statements, int count)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.count = count;
    return stmt;
}

// 创建if语句
Stmt *createIfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->as.ifStmt.condition = condition;
    stmt->as.ifStmt.thenBranch = thenBranch;
    stmt->as.ifStmt.elseBranch = elseBranch;
    return stmt;
}

// 创建while循环
Stmt *createWhileStmt(Expr *condition, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->as.whileLoop.condition = condition;
    stmt->as.whileLoop.body = body;
    return stmt;
}

Stmt *createDoWhileStmt(Stmt *body, Expr *condition)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_DO_WHILE;
    stmt->as.doWhile.body = body;
    stmt->as.doWhile.condition = condition;
    return stmt;
}

// 创建for循环
Stmt *createForStmt(Stmt *initializer, Expr *condition, Expr *increment, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_FOR;
    stmt->as.forLoop.initializer = initializer;
    stmt->as.forLoop.condition = condition;
    stmt->as.forLoop.increment = increment;
    stmt->as.forLoop.body = body;
    return stmt;
}

// 创建函数声明
Stmt *createFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnType, Stmt *body)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->as.function.name = name;
    stmt->as.function.params = params;
    stmt->as.function.paramHasVar = paramHasVar;
    stmt->as.function.paramCount = paramCount;

    // 分配并复制参数类型
    TypeAnnotation *convertedParamTypes = NULL;
    if (paramCount > 0)
    {
        convertedParamTypes = (TypeAnnotation *)malloc(paramCount * sizeof(TypeAnnotation));
        for (int i = 0; i < paramCount; i++)
        {
            convertedParamTypes[i] = paramTypes[i];
        }
    }
    stmt->as.function.paramTypes = convertedParamTypes;

    // 直接使用返回类型，不需要转换
    stmt->as.function.returnType = returnType;

    stmt->as.function.body = body;
    stmt->as.function.isStatic = false;

    return stmt;
}

Stmt *createStaticFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnType, Stmt *body)
{
    Stmt *stmt = createFunctionStmt(name, params, paramHasVar, paramTypes, paramCount, returnType, body);
    stmt->as.function.isStatic = true;
    return stmt;
}
// 创建return语句
Stmt *createReturnStmt(Token keyword, Expr *value)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.returnStmt.keyword = keyword;
    stmt->as.returnStmt.value = value;
    return stmt;
}

// 创建 switch 语句
Stmt *createSwitchStmt(Expr *discriminant, CaseStmt *cases, int caseCount)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_SWITCH;
    stmt->as.switchStmt.discriminant = discriminant;
    stmt->as.switchStmt.cases = cases;
    stmt->as.switchStmt.caseCount = caseCount;
    return stmt;
}

// 创建 break 语句
Stmt *createBreakStmt(Token keyword)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_BREAK;
    stmt->as.breakStmt.keyword = keyword;
    return stmt;
}

Stmt *createEnumStmt(Token name, EnumMember *members, int memberCount)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_ENUM;
    stmt->as.enumStmt.name = name;
    stmt->as.enumStmt.members = members;
    stmt->as.enumStmt.memberCount = memberCount;

    return stmt;
}

/**
 * 创建结构体声明语句
 */
Stmt *createStructStmt(Token name, StructField *fields, int fieldCount)
{
    Stmt *stmt = (Stmt *)malloc(sizeof(Stmt));
    if (stmt == NULL)
    {
        return NULL;
    }
    stmt->type = STMT_STRUCT;
    stmt->as.structStmt.name = name;
    stmt->as.structStmt.fields = fields;
    stmt->as.structStmt.fieldCount = fieldCount;

    return stmt;
}

/**
 * 深度复制表达式节点
 *
 * 该函数递归地复制一个表达式及其所有子表达式，包括：
 * - 二元表达式：复制左右操作数
 * - 一元表达式：复制操作数
 * - 字面量表达式：深度复制Token及其字符串内容
 * - 分组表达式：复制内部表达式
 * - 变量表达式：复制变量名Token
 * - 赋值表达式：复制变量名和值表达式
 * - 函数调用表达式：复制被调用者、参数列表和括号Token
 * - 后缀表达式：复制操作数
 * - 前缀表达式：复制操作数
 * - 数组字面量：复制所有元素表达式
 * - 数组访问：复制数组和索引表达式
 * - 数组赋值：复制数组、索引和值表达式
 * - 类型转换：复制目标类型和被转换的表达式
 *
 * @param expr 要复制的表达式指针，可以为NULL
 * @return 复制后的新表达式指针，如果输入为NULL则返回NULL，内存分配失败返回NULL
 *
 * @note 该函数会为字符串内容（如Token的lexeme和stringValue）分配新的内存
 * @note 复制失败时会自动清理已分配的内存以防止内存泄漏
 * @note 调用者负责释放返回的表达式内存
 */
Expr *copyExpr(Expr *expr)
{
    if (expr == NULL)
        return NULL;

    switch (expr->type)
    {

    // 处理二元表达式
    case EXPR_BINARY:
    {
        Expr *leftCopy = copyExpr(expr->as.binary.left);                  // 递归复制左操作数
        Expr *rightCopy = copyExpr(expr->as.binary.right);                // 递归复制右操作数
        return createBinaryExpr(leftCopy, expr->as.binary.op, rightCopy); // 创建新的二元表达式
    }

    // 处理一元表达式
    case EXPR_UNARY:
    {
        Expr *rightCopy = copyExpr(expr->as.unary.right);     // 递归复制右操作数
        return createUnaryExpr(expr->as.unary.op, rightCopy); // 创建新的一元表达式
    }

    // 处理字面量表达式
    case EXPR_LITERAL:
    {
        // 复制Token
        Token tokenCopy = expr->as.literal.value;

        // 如果是字符串类型，需要手动深度复制字符串内容
        if (tokenCopy.type == TOKEN_STRING && tokenCopy.value.stringValue != NULL)
        {
            size_t strLen = strlen(tokenCopy.value.stringValue); // 获取字符串长度
            char *strCopy = (char *)malloc(strLen + 1);          // +1 确保字符串以null结尾
            if (strCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(strCopy, tokenCopy.value.stringValue, strLen);
            strCopy[strLen] = '\0'; // 确保字符串以null结尾
            tokenCopy.value.stringValue = strCopy;
        }

        // 如果token.lexeme需要复制
        if (tokenCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(tokenCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                if (tokenCopy.type == TOKEN_STRING)
                    free(tokenCopy.value.stringValue);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, tokenCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            tokenCopy.lexeme = lexCopy;
        }

        return createLiteralExpr(tokenCopy);
    }

    // 处理分组表达式
    case EXPR_GROUPING:
    {
        Expr *exprCopy = copyExpr(expr->as.grouping.expression); // 递归复制被分组的表达式
        return createGroupingExpr(exprCopy);                     // 创建新的分组表达式
    }

    // 处理变量表达式
    case EXPR_VARIABLE:
    {
        // 复制Token
        Token nameCopy = expr->as.variable.name;
        if (nameCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(nameCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, nameCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            nameCopy.lexeme = lexCopy;
        }
        return createVariableExpr(nameCopy);
    }

    // 处理赋值表达式
    case EXPR_ASSIGN:
    {
        // 复制Token
        Token nameCopy = expr->as.assign.name;
        if (nameCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(nameCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, nameCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            nameCopy.lexeme = lexCopy;
        }

        Expr *valueCopy = copyExpr(expr->as.assign.value);
        return createAssignExpr(nameCopy, valueCopy);
    }

    // 处理函数调用表达式
    case EXPR_CALL:
    {
        Expr *calleeCopy = copyExpr(expr->as.call.callee);

        // 复制Token
        Token parenCopy = expr->as.call.paren;
        if (parenCopy.lexeme != NULL)
        {
            size_t lexLen = strlen(parenCopy.lexeme);
            char *lexCopy = (char *)malloc(lexLen + 1); // +1 for null terminator
            if (lexCopy == NULL)
            {
                freeExpr(calleeCopy);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }
            // 手动复制字符串
            memcpy(lexCopy, parenCopy.lexeme, lexLen);
            lexCopy[lexLen] = '\0'; // 确保字符串以null结尾
            parenCopy.lexeme = lexCopy;
        }

        // 复制参数
        Expr **argsCopy = NULL;
        if (expr->as.call.argCount > 0)
        {
            argsCopy = (Expr **)malloc(sizeof(Expr *) * expr->as.call.argCount);
            if (argsCopy == NULL)
            {
                freeExpr(calleeCopy);
                if (parenCopy.lexeme != NULL)
                    free(parenCopy.lexeme);
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }

            // 初始化为NULL，以便在出错时正确清理
            for (int i = 0; i < expr->as.call.argCount; i++)
                argsCopy[i] = NULL;

            // 复制每个参数
            for (int i = 0; i < expr->as.call.argCount; i++)
            {
                argsCopy[i] = copyExpr(expr->as.call.arguments[i]);
                if (argsCopy[i] == NULL)
                {
                    // 清理已复制的参数
                    for (int j = 0; j < i; j++)
                        freeExpr(argsCopy[j]);
                    free(argsCopy);
                    freeExpr(calleeCopy);
                    if (parenCopy.lexeme != NULL)
                        free(parenCopy.lexeme);
                    return NULL;
                }
            }
        }

        return createCallExpr(calleeCopy, parenCopy, argsCopy, expr->as.call.argCount);
    }

    // 处理后缀表达式
    case EXPR_POSTFIX:
    {
        Expr *operandCopy = copyExpr(expr->as.postfix.operand);
        return createPostfixExpr(operandCopy, expr->as.postfix.op);
    }

    case EXPR_PREFIX:
    {
        Expr *operandCopy = copyExpr(expr->as.prefix.operand);
        return createPrefixExpr(operandCopy, expr->as.prefix.op);
    }

    case EXPR_ARRAY_LITERAL:
    {
        Expr **elementsCopy = NULL;
        if (expr->as.arrayLiteral.elementCount > 0)
        {
            elementsCopy = (Expr **)malloc(sizeof(Expr *) * expr->as.arrayLiteral.elementCount);
            if (elementsCopy == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                return NULL;
            }

            for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++)
            {
                elementsCopy[i] = copyExpr(expr->as.arrayLiteral.elements[i]);
                if (elementsCopy[i] == NULL)
                {
                    // 清理已复制的元素
                    for (int j = 0; j < i; j++)
                        freeExpr(elementsCopy[j]);
                    free(elementsCopy);
                    return NULL;
                }
            }
        }
        return createArrayLiteralExpr(elementsCopy, expr->as.arrayLiteral.elementCount);
    }

    case EXPR_ARRAY_ACCESS:
    {
        Expr *arrayCopy = copyExpr(expr->as.arrayAccess.array);
        Expr *indexCopy = copyExpr(expr->as.arrayAccess.index);
        if (arrayCopy == NULL || indexCopy == NULL)
        {
            if (arrayCopy)
                freeExpr(arrayCopy);
            if (indexCopy)
                freeExpr(indexCopy);
            return NULL;
        }
        return createArrayAccessExpr(arrayCopy, indexCopy);
    }

    // 数组赋值
    case EXPR_ARRAY_ASSIGN:
    {
        Expr *arrayCopy = copyExpr(expr->as.arrayAssign.array); // 递归复制数组表达式
        Expr *indexCopy = copyExpr(expr->as.arrayAssign.index); // 递归复制索引表达式
        Expr *valueCopy = copyExpr(expr->as.arrayAssign.value); // 递归复制赋值表达式
        if (arrayCopy == NULL || indexCopy == NULL || valueCopy == NULL)
        {
            if (arrayCopy)
                freeExpr(arrayCopy);
            if (indexCopy)
                freeExpr(indexCopy);
            if (valueCopy)
                freeExpr(valueCopy);
            return NULL;
        }
        return createArrayAssignExpr(arrayCopy, indexCopy, valueCopy);
    }

    case EXPR_CAST:
    {
        Expr *exprCopy = copyExpr(expr->as.cast.expression);
        if (exprCopy == NULL)
        {
            return NULL;
        }
        return createCastExpr(expr->as.cast.targetType, exprCopy);
    }

    case EXPR_DOT_ACCESS:
    {
        Expr *objectCopy = copyExpr(expr->as.dotAccess.object);
        if (objectCopy == NULL)
        {
            return NULL;
        }
        return createDotAccessExpr(objectCopy, expr->as.dotAccess.member);
    }

    case EXPR_STRUCT_LITERAL:
    {
        StructFieldInit *fieldsCopy = (StructFieldInit *)malloc(sizeof(StructFieldInit) * expr->as.structLiteral.fieldCount);
        if (fieldsCopy == NULL)
        {
            return NULL;
        }
        
        for (int i = 0; i < expr->as.structLiteral.fieldCount; i++)
        {
            fieldsCopy[i].name = expr->as.structLiteral.fields[i].name;
            fieldsCopy[i].value = copyExpr(expr->as.structLiteral.fields[i].value);
            if (fieldsCopy[i].value == NULL)
            {
                // 清理已分配的内存
                for (int j = 0; j < i; j++)
                {
                    freeExpr(fieldsCopy[j].value);
                }
                free(fieldsCopy);
                return NULL;
            }
        }
        
        return createStructLiteralExpr(expr->as.structLiteral.structName, fieldsCopy, expr->as.structLiteral.fieldCount);
    }

    case EXPR_STRUCT_ASSIGN:
    {
        Expr *objectCopy = copyExpr(expr->as.structAssign.object);
        Expr *valueCopy = copyExpr(expr->as.structAssign.value);
        if (objectCopy == NULL || valueCopy == NULL)
        {
            if (objectCopy) freeExpr(objectCopy);
            if (valueCopy) freeExpr(valueCopy);
            return NULL;
        }
        return createStructAssignExpr(objectCopy, expr->as.structAssign.field, valueCopy);
    }

    default:
        fprintf(stderr, "未知的表达式类型\n");
        return NULL;
    }
}

/**
 * 创建数组字面量表达式节点
 *
 * 此函数分配内存并初始化一个新的数组字面量表达式节点。
 * 数组字面量表达式用于表示源代码中的数组初始化语法，
 * 如 [1, 2, 3] 或 ["hello", "world"]。
 *
 * @param elements 指向表达式指针数组的指针，包含数组中的所有元素表达式
 * @param elementCount 数组中元素的数量
 * @return 成功时返回指向新创建的 Expr 结构体的指针，内存分配失败时返回 NULL
 *
 * @note 调用者负责管理 elements 数组的内存生命周期
 * @note 返回的表达式节点需要在使用完毕后通过适当的释放函数进行内存清理
 */
Expr *createArrayLiteralExpr(Expr **elements, int elementCount)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr)); // 分配内存
    if (expr == NULL)                          // 检查内存分配是否成功
        return NULL;

    expr->type = EXPR_ARRAY_LITERAL;                   // 设置表达式类型为数组字面量
    expr->as.arrayLiteral.elements = elements;         // 设置数组元素指针
    expr->as.arrayLiteral.elementCount = elementCount; // 设置元素数量
    return expr;
}

/**
 * 创建数组访问表达式节点
 *
 * 该函数用于在抽象语法树中创建一个表示数组访问操作的表达式节点，
 * 例如 arr[index] 这样的表达式。
 *
 * @param array 指向数组表达式的指针，表示被访问的数组
 * @param index 指向索引表达式的指针，表示访问数组的索引
 * @return 成功时返回指向新创建的数组访问表达式节点的指针，
 *         内存分配失败时返回 NULL
 *
 * @note 调用者需要确保传入的 array 和 index 参数有效
 * @note 返回的表达式节点需要在适当时机释放内存以避免内存泄漏
 */
Expr *createArrayAccessExpr(Expr *array, Expr *index)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr)); // 分配内存
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_ARRAY_ACCESS;     // 设置表达式类型为数组访问
    expr->as.arrayAccess.array = array; // 设置数组表达式
    expr->as.arrayAccess.index = index; // 设置索引表达式
    return expr;
}

/**
 * 创建数组赋值表达式节点
 *
 * 此函数分配内存并初始化一个新的表达式节点，用于表示数组赋值操作（如 array[index] = value）。
 *
 * @param array 指向数组表达式的指针，表示被赋值的数组
 * @param index 指向索引表达式的指针，表示数组的索引位置
 * @param value 指向值表达式的指针，表示要赋给数组元素的值
 *
 * @return 成功时返回指向新创建的表达式节点的指针；
 *         内存分配失败时返回 NULL
 *
 * @note 调用者负责释放返回的表达式节点及其所有子表达式的内存
 * @note 传入的 array、index 和 value 参数将被直接存储，不会进行深拷贝
 */
Expr *createArrayAssignExpr(Expr *array, Expr *index, Expr *value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr)); // 分配内存
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_ARRAY_ASSIGN;     // 设置表达式类型为数组赋值
    expr->as.arrayAssign.array = array; // 设置数组表达式
    expr->as.arrayAssign.index = index; // 设置索引表达式
    expr->as.arrayAssign.value = value; // 设置赋值表达式
    return expr;
}

/**
 * 创建类型转换表达式节点
 *
 * 此函数分配内存并初始化一个新的类型转换表达式节点，用于表示将一个表达式
 * 转换为指定的目标类型。
 *
 * @param targetType 目标类型，表达式将被转换为此类型
 * @param expression 要进行类型转换的源表达式
 * @return 成功时返回指向新创建的表达式节点的指针，内存分配失败时返回 NULL
 *
 * @note 调用者负责管理返回的表达式节点的内存，使用完毕后应调用相应的释放函数
 * @note 如果内存分配失败，函数返回 NULL，调用者应检查返回值
 */
Expr *createCastExpr(BaseType targetType, Expr *expression)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_CAST;
    expr->as.cast.targetType = targetType;
    expr->as.cast.expression = expression;
    return expr;
}

/**
 * 创建点访问表达式
 */
Expr *createDotAccessExpr(Expr *object, Token member)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_DOT_ACCESS;
    expr->as.dotAccess.object = object;
    expr->as.dotAccess.member = member;
    return expr;
}

/**
 * 创建结构体字面量表达式
 */
Expr *createStructLiteralExpr(Token structName, StructFieldInit *fields, int fieldCount)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_STRUCT_LITERAL;
    expr->as.structLiteral.structName = structName;
    expr->as.structLiteral.fields = fields;
    expr->as.structLiteral.fieldCount = fieldCount;
    return expr;
}

/**
 * 创建结构体字段赋值表达式
 */
Expr *createStructAssignExpr(Expr *object, Token field, Expr *value)
{
    Expr *expr = (Expr *)malloc(sizeof(Expr));
    if (expr == NULL)
        return NULL;

    expr->type = EXPR_STRUCT_ASSIGN;
    expr->as.structAssign.object = object;
    expr->as.structAssign.field = field;
    expr->as.structAssign.value = value;
    return expr;
}

/**
 * 释放表达式节点及其所有子节点的内存
 *
 * 该函数递归地释放表达式树中的所有节点，包括：
 * - 二元表达式：释放左右操作数
 * - 一元表达式：释放操作数
 * - 分组表达式：释放内部表达式
 * - 赋值表达式：释放赋值值
 * - 函数调用：释放被调用者和所有参数
 * - 前缀/后缀表达式：释放操作数
 * - 数组字面量：释放所有元素
 * - 数组访问：释放数组和索引表达式
 * - 数组赋值：释放数组、索引和赋值值
 * - 类型转换：释放被转换的表达式
 * - 字面量和变量：无需额外释放
 *
 * @param expr 要释放的表达式节点指针，可以为NULL（安全处理）
 *
 * @note 该函数会递归释放整个表达式树，确保没有内存泄漏
 * @note 对NULL指针调用是安全的
 * @warning 释放后不应再使用该表达式指针
 */
void freeExpr(Expr *expr)
{
    if (expr == NULL)
        return;

    switch (expr->type)
    {
    case EXPR_BINARY:
        freeExpr(expr->as.binary.left);
        freeExpr(expr->as.binary.right);
        break;
    case EXPR_UNARY:
        freeExpr(expr->as.unary.right);
        break;
    case EXPR_GROUPING:
        freeExpr(expr->as.grouping.expression);
        break;
    case EXPR_ASSIGN:
        freeExpr(expr->as.assign.value);
        break;
    case EXPR_CALL:
        freeExpr(expr->as.call.callee);
        for (int i = 0; i < expr->as.call.argCount; i++)
        {
            freeExpr(expr->as.call.arguments[i]);
        }
        free(expr->as.call.arguments);
        break;
    case EXPR_POSTFIX:
        freeExpr(expr->as.postfix.operand);
        break;
    case EXPR_PREFIX:
        freeExpr(expr->as.prefix.operand);
        break;
    case EXPR_ARRAY_LITERAL:
        for (int i = 0; i < expr->as.arrayLiteral.elementCount; i++)
        {
            freeExpr(expr->as.arrayLiteral.elements[i]);
        }
        free(expr->as.arrayLiteral.elements);
        break;

    case EXPR_ARRAY_ACCESS:
        freeExpr(expr->as.arrayAccess.array);
        freeExpr(expr->as.arrayAccess.index);
        break;

    case EXPR_ARRAY_ASSIGN:
        freeExpr(expr->as.arrayAssign.array);
        freeExpr(expr->as.arrayAssign.index);
        freeExpr(expr->as.arrayAssign.value);
        break;
    case EXPR_CAST:
        if (expr->as.cast.expression != NULL)
        {
            freeExpr(expr->as.cast.expression);
        }
        break;
    case EXPR_DOT_ACCESS:
        if (expr->as.dotAccess.object != NULL)
        {
            freeExpr(expr->as.dotAccess.object);
        }
        break;
    case EXPR_STRUCT_LITERAL:
        if (expr->as.structLiteral.fields != NULL)
        {
            for (int i = 0; i < expr->as.structLiteral.fieldCount; i++)
            {
                if (expr->as.structLiteral.fields[i].value != NULL)
                {
                    freeExpr(expr->as.structLiteral.fields[i].value);
                }
            }
            free(expr->as.structLiteral.fields);
        }
        break;
    case EXPR_STRUCT_ASSIGN:
        if (expr->as.structAssign.object != NULL)
        {
            freeExpr(expr->as.structAssign.object);
        }
        if (expr->as.structAssign.value != NULL)
        {
            freeExpr(expr->as.structAssign.value);
        }
        break;
    case EXPR_LITERAL:
    case EXPR_VARIABLE:
        // 这些节点没有需要释放的指针
        break;
    }

    free(expr);
}

// 释放语句节点内存
void freeStmt(Stmt *stmt)
{
    if (stmt == NULL)
        return;

    switch (stmt->type)
    {
    case STMT_EXPRESSION:
        freeExpr(stmt->as.expression.expression);
        break;
    case STMT_VAR:
        freeExpr(stmt->as.var.initializer);
        break;
    case STMT_CONST: // 添加常量声明的内存释放
        if (stmt->as.constStmt.initializer != NULL)
        {
            freeExpr(stmt->as.constStmt.initializer);
        }
        break;
    case STMT_MULTI_VAR:
        if (stmt->as.multiVar.initializer != NULL)
        {
            freeExpr(stmt->as.multiVar.initializer);
        }
        if (stmt->as.multiVar.names != NULL)
        {
            free(stmt->as.multiVar.names);
        }
        break;
    case STMT_MULTI_CONST:
        if (stmt->as.multiConst.initializers != NULL)
        {
            // 只释放实际存在的初始值表达式，避免重复释放
            for (int i = 0; i < stmt->as.multiConst.initializerCount; i++)
            {
                if (stmt->as.multiConst.initializers[i] != NULL)
                {
                    freeExpr(stmt->as.multiConst.initializers[i]);
                }
            }
            free(stmt->as.multiConst.initializers);
        }
        if (stmt->as.multiConst.names != NULL)
        {
            free(stmt->as.multiConst.names);
        }
        break;
    case STMT_BLOCK:
        for (int i = 0; i < stmt->as.block.count; i++)
        {
            freeStmt(stmt->as.block.statements[i]);
        }
        free(stmt->as.block.statements);
        break;
    case STMT_IF:
        freeExpr(stmt->as.ifStmt.condition);
        freeStmt(stmt->as.ifStmt.thenBranch);
        if (stmt->as.ifStmt.elseBranch)
        {
            freeStmt(stmt->as.ifStmt.elseBranch);
        }
        break;
    case STMT_WHILE:
        freeExpr(stmt->as.whileLoop.condition);
        freeStmt(stmt->as.whileLoop.body);
        break;

    case STMT_DO_WHILE:
        if (stmt->as.doWhile.body != NULL)
        {
            freeStmt(stmt->as.doWhile.body);
        }
        if (stmt->as.doWhile.condition != NULL)
        {
            freeExpr(stmt->as.doWhile.condition);
        }
        break;

    case STMT_FOR:
        if (stmt->as.forLoop.initializer)
        {
            freeStmt(stmt->as.forLoop.initializer);
        }
        if (stmt->as.forLoop.condition)
        {
            freeExpr(stmt->as.forLoop.condition);
        }
        if (stmt->as.forLoop.increment)
        {
            freeExpr(stmt->as.forLoop.increment);
        }
        freeStmt(stmt->as.forLoop.body);
        break;
    case STMT_FUNCTION:
        if (stmt->as.function.params)
        {
            free(stmt->as.function.params);
        }
        if (stmt->as.function.paramHasVar)
        {
            free(stmt->as.function.paramHasVar);
        }
        if (stmt->as.function.paramTypes)
        {
            free(stmt->as.function.paramTypes);
        }
        if (stmt->as.function.body)
        {
            freeStmt(stmt->as.function.body);
        }
        break;
    case STMT_RETURN:
        if (stmt->as.returnStmt.value)
        {
            freeExpr(stmt->as.returnStmt.value);
        }
        break;
    case STMT_SWITCH:
        if (stmt->as.switchStmt.discriminant != NULL)
        {
            freeExpr(stmt->as.switchStmt.discriminant);
        }
        if (stmt->as.switchStmt.cases != NULL)
        {
            for (int i = 0; i < stmt->as.switchStmt.caseCount; i++)
            {
                if (stmt->as.switchStmt.cases[i].value != NULL)
                {
                    freeExpr(stmt->as.switchStmt.cases[i].value);
                }
                if (stmt->as.switchStmt.cases[i].body != NULL)
                {
                    freeStmt(stmt->as.switchStmt.cases[i].body);
                }
            }
            free(stmt->as.switchStmt.cases);
        }
        break;
    case STMT_ENUM:
        if (stmt->as.enumStmt.members != NULL)
        {
            for (int i = 0; i < stmt->as.enumStmt.memberCount; i++)
            {
                if (stmt->as.enumStmt.members[i].value != NULL)
                {
                    freeExpr(stmt->as.enumStmt.members[i].value);
                }
            }
            free(stmt->as.enumStmt.members);
        }
        break;
    case STMT_STRUCT:
        if (stmt->as.structStmt.fields != NULL)
        {
            free(stmt->as.structStmt.fields);
        }
        break;
    case STMT_BREAK:
        // break 语句没有需要释放的内存
        break;
    }

    free(stmt);
}