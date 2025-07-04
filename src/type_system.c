#include "type_system.h"
#include "lexer.h"
#include "value.h"

// 类型注解转字符串
const char *annotationToString(TypeAnnotation type)
{
    if (type.kind == TYPE_ARRAY)
    {
        // 处理数组类型，这里简化处理
        switch (type.as.array.elementType)
        {
        case TYPE_INT:
            return "int[]";
        case TYPE_FLOAT:
            return "float[]";
        case TYPE_STRING:
            return "string[]";
        case TYPE_BOOL:
            return "boolean[]";
        default:
            return "unknown[]";
        }
    }

    // 处理简单类型
    switch (type.as.simple)
    {
    case TYPE_ANY:
        return "any";
    case TYPE_VOID:
        return "void";
    case TYPE_INT:
        return "int";
    case TYPE_FLOAT:
        return "float";
    case TYPE_STRING:
        return "string";
    case TYPE_BOOL:
        return "boolean";
    case TYPE_FUNCTION:
        return "function";
    default:
        return "unknown";
    }
}

// 从词法标记转换为类型注解
TypeAnnotation tokenToTypeAnnotation(int tokenType)
{
    TypeAnnotation annotation;
    annotation.kind = TYPE_SIMPLE;

    switch (tokenType)
    {
    case TOKEN_VOID:
        annotation.as.simple = TYPE_VOID;
        break;
    case TOKEN_INT:
        annotation.as.simple = TYPE_INT;
        break;
    case TOKEN_FLOAT_TYPE:
        annotation.as.simple = TYPE_FLOAT;
        break;
    case TOKEN_STRING_TYPE:
        annotation.as.simple = TYPE_STRING;
        break;
    case TOKEN_BOOL:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        annotation.as.simple = TYPE_BOOL;
        break;
    default:
        annotation.as.simple = TYPE_ANY;
        break;
    }

    return annotation;
}