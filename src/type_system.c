#include "type_system.h"
#include "lexer.h"
#include "value.h"

// 类型注解转字符串
const char *annotationToString(TypeAnnotation type)
{
    switch (type)
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
    switch (tokenType)
    {
    case TOKEN_VOID:
        return TYPE_VOID;
    case TOKEN_INT:
        return TYPE_INT;
    case TOKEN_FLOAT:
        return TYPE_FLOAT;
    case TOKEN_STRING:
        return TYPE_STRING;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        return TYPE_BOOL;
    default:
        return TYPE_ANY;
    }
}