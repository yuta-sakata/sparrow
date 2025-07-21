#ifndef SPARROW_TYPE_SYSTEM_H
#define SPARROW_TYPE_SYSTEM_H
#include <stdbool.h>

struct Expr;

// 基本数据类型注解
typedef enum
{
    TYPE_ANY,     // 未指定类型
    TYPE_VOID,    // void类型
    TYPE_INT,     // 整数类型
    TYPE_FLOAT,   // 浮点数类型
    TYPE_DOUBLE,  // 双精度浮点数类型
    TYPE_STRING,  // 字符串类型
    TYPE_BOOL,    // 布尔类型
    TYPE_FUNCTION, // 函数类型
    TYPE_ENUM,    // 枚举类型
    TYPE_STRUCT   // 结构体类型
} BaseType;

// 类型注解结构
typedef struct TypeAnnotation
{
    enum
    {
        TYPE_SIMPLE,
        TYPE_ARRAY
    } kind;
    union
    {
        BaseType simple; // 简单类型
        struct
        {
            BaseType elementType; // 数组元素类型
            struct Expr *size;    // 数组大小（NULL表示动态数组）
        } array;
    } as;
} TypeAnnotation;

const char *annotationToString(TypeAnnotation type);// 获取类型注解的字符串表示
TypeAnnotation tokenToTypeAnnotation(int tokenType);// 将令牌类型转换为类型注解

#endif // SPARROW_TYPE_SYSTEM_H