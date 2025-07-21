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
    EXPR_CAST,          // 类型转换表达式
    EXPR_DOT_ACCESS,    // 点访问表达式（如 obj.member）
    EXPR_STRUCT_LITERAL, // 结构体字面量
    EXPR_STRUCT_ASSIGN,  // 结构体字段赋值
} ExprType;

// 语句类型
typedef enum
{
    STMT_EXPRESSION, // 表达式语句
    STMT_VAR,        // 变量声明
    STMT_CONST,      // 常量声明
    STMT_MULTI_VAR,  // 多变量声明
    STMT_MULTI_CONST, // 多常量声明
    STMT_BLOCK,      // 代码块
    STMT_IF,         // if语句
    STMT_WHILE,      // while循环
    STMT_FOR,        // for循环
    STMT_FUNCTION,   // 函数声明
    STMT_RETURN,     // return语句
    STMT_SWITCH,     // switch语句
    STMT_BREAK,      // break语句
    STMT_DO_WHILE,   // do-while循环
    STMT_ENUM,       // 枚举声明
    STMT_STRUCT,     // 结构体声明
} StmtType;

// 枚举成员结构
typedef struct
{
    Token name;  // 枚举成员名称
    Expr *value; // 枚举成员值（可选）
} EnumMember;

// 枚举声明语句结构
typedef struct
{
    Token name;          // 枚举类型名称
    EnumMember *members; // 枚举成员数组
    int memberCount;     // 成员数量
} EnumStmt;

// 结构体字段结构
typedef struct
{
    Token name;          // 字段名称
    TypeAnnotation type; // 字段类型
} StructField;

// 结构体声明语句结构
typedef struct
{
    Token name;            // 结构体类型名称
    StructField *fields;   // 字段数组
    int fieldCount;        // 字段数量
} StructStmt;

// 常量声明语句结构
typedef struct
{
    Token name;          // 常量名
    TypeAnnotation type; // 类型信息
    Expr *initializer;   // 常量必须有初始值
    bool isStatic;       // 是否为静态常量
} ConstStmt;

// 多变量声明语句结构
typedef struct
{
    Token *names;        // 变量名数组
    int count;           // 变量数量
    TypeAnnotation type; // 共享的类型
    Expr *initializer;   // 共享的初始值
    bool isStatic;       // 是否为静态变量
} MultiVarStmt;

// 多常量声明语句结构
typedef struct
{
    Token *names;        // 常量名数组
    int count;           // 常量数量
    TypeAnnotation type; // 共享的类型
    Expr **initializers; // 初始值数组（每个常量一个初始值）
    int initializerCount; // 初始值数量
    bool isStatic;       // 是否为静态常量
} MultiConstStmt;

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

// 类型转换表达式
typedef struct
{
    BaseType targetType; // 目标类型
    Expr *expression;    // 被转换的表达式
} CastExpr;

// 点访问表达式
typedef struct
{
    Expr *object;   // 被访问的对象（如枚举名）
    Token member;   // 成员名称
} DotAccessExpr;

// 结构体字段初始化
typedef struct
{
    Token name;     // 字段名称
    Expr *value;    // 字段值
} StructFieldInit;

// 结构体字面量表达式
typedef struct
{
    Token structName;            // 结构体类型名称
    StructFieldInit *fields;     // 字段初始化数组
    int fieldCount;              // 字段数量
} StructLiteralExpr;

// 结构体字段赋值表达式
typedef struct
{
    Expr *object;   // 被赋值的结构体对象
    Token field;    // 字段名称
    Expr *value;    // 赋值的值
} StructAssignExpr;

// 表达式结构
typedef struct Expr
{
    ExprType type; // 表达式类型
    union
    {
        BinaryExpr binary;             // 二元表达式
        UnaryExpr unary;               // 一元表达式
        LiteralExpr literal;           // 字面量表达式
        PostfixExpr postfix;           // 后缀表达式
        PrefixExpr prefix;             // 前缀表达式
        GroupingExpr grouping;         // 分组表达式
        VariableExpr variable;         // 变量引用
        AssignExpr assign;             // 赋值表达式
        CallExpr call;                 // 函数调用
        ArrayLiteralExpr arrayLiteral; // 数组字面量表达式
        ArrayAccessExpr arrayAccess;   // 数组访问表达式
        ArrayAssignExpr arrayAssign;   // 数组赋值表达式
        CastExpr cast;                 // 类型转换表达式
        DotAccessExpr dotAccess;       // 点访问表达式
        StructLiteralExpr structLiteral; // 结构体字面量表达式
        StructAssignExpr structAssign;   // 结构体字段赋值表达式
    } as;
} Expr;

// 表达式语句
typedef struct
{
    Expr *expression; // 表达式
} ExpressionStmt;

// 变量声明
typedef struct
{
    Token name;
    TypeAnnotation type; // 类型信息
    Expr *initializer;   // 初始值，可以为NULL
    bool isStatic;       // 是否为静态变量
} VarStmt;

// 代码块
typedef struct
{
    Stmt **statements; // 语句数组
    int count;         // 语句数量
} BlockStmt;

// if语句
typedef struct
{
    Expr *condition;  // 条件表达式
    Stmt *thenBranch; // 真分支
    Stmt *elseBranch; // 假分支，可以为NULL
} IfStmt;

// while循环
typedef struct
{
    Expr *condition; // 条件表达式
    Stmt *body;      // 循环体
} WhileStmt;

// for循环
typedef struct
{
    Stmt *initializer; // 循环初始化语句，可以为NULL
    Expr *condition;   // 循环条件，可以为NULL
    Expr *increment;   // 循环增量表达式，可以为NULL
    Stmt *body;        // 循环体
} ForStmt;

// 函数声明
typedef struct
{
    Token name;                 // 函数名
    Token *params;              // 参数名数组
    bool *paramHasVar;          // 参数是否为可变参数
    TypeAnnotation *paramTypes; // 参数类型数组
    TypeAnnotation returnType;  // 返回类型
    int paramCount;             // 参数数量
    struct Stmt *body;          // 函数体
    bool isStatic;              // 是否为静态函数
} FunctionStmt;

// return语句
typedef struct
{
    Token keyword; // 用于错误报告的位置信息
    Expr *value;   // 可能为NULL
} ReturnStmt;

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

typedef struct DoWhileStmt
{
    Stmt *body;      // 循环体
    Expr *condition; // 循环条件
} DoWhileStmt;

// 语句结构
struct Stmt
{
    StmtType type;
    union
    {
        ExpressionStmt expression; // 表达式语句
        VarStmt var;               // 变量声明
        ConstStmt constStmt;       // 常量声明
        MultiVarStmt multiVar;     // 多变量声明
        MultiConstStmt multiConst; // 多常量声明
        BlockStmt block;           // 代码块
        IfStmt ifStmt;             // if语句
        WhileStmt whileLoop;       // while循环
        ForStmt forLoop;           // for循环
        FunctionStmt function;     // 函数声明
        ReturnStmt returnStmt;     // return语句
        SwitchStmt switchStmt;     // switch语句
        BreakStmt breakStmt;       // break语句
        DoWhileStmt doWhile;       // do-while循环
        EnumStmt enumStmt;         // 枚举声明
        StructStmt structStmt;     // 结构体声明
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
Expr *createCastExpr(BaseType targetType, Expr *expression);
Expr *createDotAccessExpr(Expr *object, Token member);
Expr *createStructLiteralExpr(Token structName, StructFieldInit *fields, int fieldCount);
Expr *createStructAssignExpr(Expr *object, Token field, Expr *value);

// 创建语句节点的函数
Stmt *createExpressionStmt(Expr *expression);
Stmt *createVarStmt(Token name, TypeAnnotation type, Expr *initializer);
Stmt *createConstStmt(Token name, TypeAnnotation type, Expr *initializer);
Stmt *createMultiVarStmt(Token *names, int count, TypeAnnotation type, Expr *initializer);
Stmt *createMultiConstStmt(Token *names, int count, TypeAnnotation type, Expr **initializers, int initializerCount);
Stmt *createBlockStmt(Stmt **statements, int count);
Stmt *createIfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
Stmt *createWhileStmt(Expr *condition, Stmt *body);
Stmt *createDoWhileStmt(Stmt *body, Expr *condition);
Stmt *createForStmt(Stmt *initializer, Expr *condition, Expr *increment, Stmt *body);
Stmt *createFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnTypeToken, Stmt *body);
Stmt *createReturnStmt(Token keyword, Expr *value);
Stmt *createSwitchStmt(Expr *discriminant, CaseStmt *cases, int caseCount);
Stmt *createBreakStmt(Token keyword);
Stmt *createEnumStmt(Token name, EnumMember *members, int memberCount);
Stmt *createStructStmt(Token name, StructField *fields, int fieldCount);
Stmt *createStaticVarStmt(Token name, TypeAnnotation type, Expr *initializer);
Stmt *createStaticFunctionStmt(Token name, Token *params, bool *paramHasVar, TypeAnnotation *paramTypes, int paramCount, TypeAnnotation returnType, Stmt *body);



// 释放AST节点内存
void freeExpr(Expr *expr);
void freeStmt(Stmt *stmt);

#endif // SPARROW_AST_H