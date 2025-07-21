#include "type_system.h"
#include "lexer.h"
#include "value.h"

static const char *baseTypeToString(BaseType type);

// 类型注解转字符串
const char *annotationToString(TypeAnnotation type)
{
    static char buffer[64];

    if (type.kind == TYPE_ARRAY)
    {
        snprintf(buffer, sizeof(buffer), "%s[]",
                 baseTypeToString(type.as.array.elementType));
        return buffer;
    }

    // 简单类型
    return baseTypeToString(type.as.simple);
}

static const char *baseTypeToString(BaseType type)
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
    case TYPE_DOUBLE:
        return "double";
    case TYPE_STRING:
        return "string";
    case TYPE_BOOL:
        return "bool";
    case TYPE_FUNCTION:
        return "function";
    case TYPE_ENUM:
        return "enum";
    case TYPE_STRUCT:
        return "struct";
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
    case TOKEN_DOUBLE:
        annotation.as.simple = TYPE_DOUBLE;
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
    case TOKEN_STRUCT:
        annotation.as.simple = TYPE_STRUCT;
        break;
    default:
        annotation.as.simple = TYPE_ANY;
        break;
    }

    return annotation;
}