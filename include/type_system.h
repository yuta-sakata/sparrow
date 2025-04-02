#ifndef SPARROW_TYPE_SYSTEM_H
#define SPARROW_TYPE_SYSTEM_H
#include <stdbool.h>

// 定义类型注解枚举
typedef enum
{
    TYPE_ANY,     // 未指定类型
    TYPE_VOID,    // void类型
    TYPE_INT,     // 整数类型
    TYPE_FLOAT,   // 浮点数类型
    TYPE_STRING,  // 字符串类型
    TYPE_BOOL,    // 布尔类型
    TYPE_FUNCTION // 函数类型
} TypeAnnotation;

// 类型操作函数
const char *annotationToString(TypeAnnotation type);
TypeAnnotation tokenToTypeAnnotation(int tokenType);

#endif // SPARROW_TYPE_SYSTEM_H