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
    EXPR_BINARY,   // 二元表达式
    EXPR_UNARY,    // 一元表达式
    EXPR_POSTFIX,    // 后缀表达式
    EXPR_LITERAL,  // 字面量
    EXPR_GROUPING, // 分组表达式
    EXPR_VARIABLE, // 变量引用
    EXPR_ASSIGN,   // 赋值
    EXPR_CALL,     // 函数调用
} ExprType;

// 语句类型
typedef enum
{
    STMT_EXPRESSION, // 表达式语句
    STMT_VAR,        // 变量声明
    STMT_MULTI_VAR,  // 多变量声明
    STMT_BLOCK,      // 代码块
    STMT_IF,         // if语句
    STMT_WHILE,      // while循环
    STMT_FOR,        // for循环
    STMT_FUNCTION,   // 函数声明
    STMT_RETURN,     // return语句
} StmtType;

// 多变量声明语句结构
typedef struct {
    Token *names;      // 变量名数组
    int count;         // 变量数量
    Token type;        // 共享的类型
    Expr *initializer; // 共享的初始值
} MultiVarStmt;

// 二元表达式
typedef struct
{
    Expr *left;
    TokenType op;
    Expr *right;
} BinaryExpr;

// 一元表达式
typedef struct
{
    TokenType op;
    Expr *right;
} UnaryExpr;

// 后缀表达式
typedef struct
{
    Expr *operand;  // 被操作的变量
    TokenType op;   // 运算符 (++ 或 --)
} PostfixExpr;

// 字面量表达式
typedef struct
{
    Token value;
} LiteralExpr;

// 分组表达式
typedef struct
{
    Expr *expression;
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

// 表达式结构
struct Expr
{
    ExprType type;
    union
    {
        BinaryExpr binary;
        UnaryExpr unary;
        LiteralExpr literal;
        PostfixExpr postfix;
        GroupingExpr grouping;
        VariableExpr variable;
        AssignExpr assign;
        CallExpr call;
    } as;
};

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
        MultiVarStmt multiVar;
        BlockStmt block;
        IfStmt ifStmt;
        WhileStmt whileLoop;
        ForStmt forLoop;
        FunctionStmt function;
        ReturnStmt returnStmt;
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
Expr *copyExpr(Expr *expr);

// 创建语句节点的函数
Stmt *createExpressionStmt(Expr *expression);
Stmt *createVarStmt(Token name, Token type, Expr *initializer);
Stmt *createMultiVarStmt(Token *names, int count, Token type, Expr *initializer);
Stmt *createBlockStmt(Stmt **statements, int count);
Stmt *createIfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
Stmt *createWhileStmt(Expr *condition, Stmt *body);
Stmt *createForStmt(Stmt *initializer, Expr *condition, Expr *increment, Stmt *body);
Stmt *createFunctionStmt(Token name, Token *params, bool *paramHasVar, Token *paramTypes,int paramCount, Token returnTypeToken, Stmt *body);
Stmt *createReturnStmt(Token keyword, Expr *value);

// 释放AST节点内存
void freeExpr(Expr *expr);
void freeStmt(Stmt *stmt);

#endif // SPARROW_AST_H