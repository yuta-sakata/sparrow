#ifndef SPARROW_AST_H
#define SPARROW_AST_H

#include <stdlib.h>
#include "lexer.h"
#include "type_system.h"

// 前向声明
typedef struct Expr Expr;
typedef struct Stmt Stmt;

// 表达式类型
typedef enum
{
    EXPR_BINARY,        // 二元表达式
    EXPR_UNARY,         // 一元表达式
    EXPR_POSTFIX,       // 后缀表达式
    EXPR_PREFIX,        // 前缀表达式
    EXPR_LITERAL,       // 字面量
    EXPR_GROUPING,      // 分组表达式
    EXPR_VARIABLE,      // 变量引用
    EXPR_ASSIGN,        // 赋值
    EXPR_CALL,          // 函数调用
    EXPR_ARRAY_LITERAL, // 数组字面量
    EXPR_ARRAY_ACCESS,  // 数组访问
    EXPR_ARRAY_ASSIGN,  // 数组赋值
    STMT_SWITCH,        // switch语句
    STMT_BREAK,         // break语句
} ExprType;

// 语句类型
typedef enum
{
    STMT_EXPRESSION, // 表达式语句
    STMT_VAR,        // 变量声明
    STMT_CONST,      // 常量声明
    STMT_MULTI_VAR,  // 多变量声明
    STMT_BLOCK,      // 代码块
    STMT_IF,         // if语句
    STMT_WHILE,      // while循环
    STMT_FOR,        // for循环
    STMT_FUNCTION,   // 函数声明
    STMT_RETURN,     // return语句
} StmtType;

// case 语句结构
typedef struct
{
    Expr *value; // case 值表达式，NULL 表示 default
    Stmt *body;  // case 体
} CaseStmt;

// switch 语句结构
typedef struct
{
    Expr *discriminant; // switch 表达式
    CaseStmt *cases;    // case 数组
    int caseCount;      // case 数量
} SwitchStmt;

// break 语句结构
typedef struct
{
    Token keyword; // break 关键字
} BreakStmt;

// 常量声明语句结构
typedef struct
{
    Token name;
    TypeAnnotation type;
    Expr *initializer; // 常量必须有初始值
} ConstStmt;

// 多变量声明语句结构
typedef struct
{
    Token *names;        // 变量名数组
    int count;           // 变量数量
    TypeAnnotation type; // 共享的类型
    Expr *initializer;   // 共享的初始值
} MultiVarStmt;

// 二元表达式
typedef struct
{
    Expr *left;   // 左操作数
    TokenType op; // 运算符
    Expr *right;  // 右操作数
} BinaryExpr;

// 一元表达式
typedef struct
{
    TokenType op; // 运算符
    Expr *right;  // 被操作的表达式
} UnaryExpr;

// 后缀表达式
typedef struct
{
    Expr *operand; // 被操作的变量
    TokenType op;  // 运算符 (++ 或 --)
} PostfixExpr;

// 前缀表达式
typedef struct
{
    Expr *operand; // 被操作的表达式
    TokenType op;  // 运算符 (++ 或 --)
} PrefixExpr;

// 字面量表达式
typedef struct
{
    Token value; // 字面量值
} LiteralExpr;

// 分组表达式
typedef struct
{
    Expr *expression; // 被分组的表达式
} GroupingExpr;

// 变量引用
typedef struct
{
    Token name;
} VariableExpr;

// 赋值表达式
typedef struct
{
    Token name;
    Expr *value;
} AssignExpr;

// 函数调用
typedef struct
{
    Expr *callee;
    Token paren; // 用于错误报告的位置信息
    Expr **arguments;
    int argCount;
} CallExpr;

// 数组字面量表达式
typedef struct
{
    Expr **elements;  // 数组元素
    int elementCount; // 元素数量
} ArrayLiteralExpr;

// 数组访问表达式
typedef struct
{
    Expr *array; // 被访问的数组
    Expr *index; // 索引表达式
} ArrayAccessExpr;

// 数组赋值表达式
typedef struct
{
    Expr *array; // 被赋值的数组表达式
    Expr *index; // 索引表达式
    Expr *value; // 赋值的值
} ArrayAssignExpr;

// 表达式结构
typedef struct Expr
{
    ExprType type;
    union
    {
        BinaryExpr binary;
        UnaryExpr unary;
        LiteralExpr literal;
        PostfixExpr postfix;
        PrefixExpr prefix;
        GroupingExpr grouping;
        VariableExpr variable;
        AssignExpr assign;
        CallExpr call;
        ArrayLiteralExpr arrayLiteral;
        ArrayAccessExpr arrayAccess;
        ArrayAssignExpr arrayAssign;
    } as;
} Expr;

// 表达式语句
typedef struct
{
    Expr *expression;
} ExpressionStmt;

// 变量声明
typedef struct
{
    Token name;
    TypeAnnotation type; // 类型信息
    Expr *initializer;
} VarStmt;

// 代码块
typedef struct
{
    Stmt **statements;
    int count;
} BlockStmt;

// if语句
typedef struct
{
    Expr *condition;
    Stmt *thenBranch;
    Stmt *elseBranch; // 可能为NULL
} IfStmt;

// while循环
typedef struct
{
    Expr *condition;
    Stmt *body;
} WhileStmt;

// for循环
typedef struct
{
    Stmt *initializer; // 可能为NULL
    Expr *condition;   // 可能为NULL
    Expr *increment;   // 可能为NULL
    Stmt *body;
} ForStmt;

// 函数声明
typedef struct
{
    Token name;
    Token *params;
    bool *paramHasVar;
    TypeAnnotation *paramTypes;
    TypeAnnotation returnType;
    int paramCount;
    struct Stmt *body;
} FunctionStmt;

// return语句
typedef struct
{
    Token keyword; // 用于错误报告的位置信息
    Expr *value;   // 可能为NULL
} ReturnStmt;

// 语句结构
struct Stmt
{
    StmtType type;
    union
    {
        ExpressionStmt expression;
        VarStmt var;
        ConstStmt constStmt;
        MultiVarStmt multiVar;
        BlockStmt block;
        IfStmt ifStmt;
        WhileStmt whileLoop;
        ForStmt forLoop;
        FunctionStmt function;
        ReturnStmt returnStmt;
        SwitchStmt switchStmt;
        BreakStmt breakStmt;
    } as;
};

// 创建表达式节点的函数
Expr *createBinaryExpr(Expr *left, TokenType op, Expr *right);
Expr *createUnaryExpr(TokenType op, Expr *right);
Expr *createLiteralExpr(Token value);
Expr *createGroupingExpr(Expr *expression);
Expr *createVariableExpr(Token name);
Expr *createAssignExpr(Token name, Expr *value);
Expr *createCallExpr(Expr *callee, Token paren, Expr **arguments, int argCount);
Expr *createPostfixExpr(Expr *operand, TokenType op);
Expr *createPrefixExpr(Expr *operand, TokenType op);
Expr *copyExpr(Expr *expr);
Expr *createArrayLiteralExpr(Expr **elements, int count);
Expr *createArrayAccessExpr(Expr *array, Expr *index);
Expr *createArrayAssignExpr(Expr *array, Expr *index, Expr *value);

// 创建语句节点的函数
Stmt *createExpressionStmt(Expr *expression);
Stmt *createVarStmt(Token name, TypeAnnotation type, Expr *initializer);
Stmt *createConstStmt(Token name, TypeAnnotation type, Expr *initializer);
Stmt *createMultiVarStmt(Token *names, int count, TypeAnnotation type, Expr *initializer);
Stmt *createBlockStmt(Stmt **statements, int count);
Stmt *createIfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
Stmt *createWhileStmt(Expr *condition, Stmt *body);
Stmt *createForStmt(Stmt *initializer, Expr *condition, Expr *increment, Stmt *body);
Stmt *createFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnTypeToken, Stmt *body);
Stmt *createReturnStmt(Token keyword, Expr *value);
Stmt *createSwitchStmt(Expr *discriminant, CaseStmt *cases, int caseCount);
Stmt *createBreakStmt(Token keyword);

// 释放AST节点内存
void freeExpr(Expr *expr);
void freeStmt(Stmt *stmt);

#endif // SPARROW_AST_H