#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/parser/type_parser.h"

TypeAnnotation parseTypeAnnotation(Parser *parser)
{
    TypeAnnotation type;

    // 解析基本类型
    BaseType baseType = TYPE_ANY;

    if (match(parser, TOKEN_INT))
    {
        baseType = TYPE_INT;
    }
    else if (match(parser, TOKEN_FLOAT_TYPE))
    {
        baseType = TYPE_FLOAT;
    }
    else if (match(parser, TOKEN_STRING_TYPE))
    {
        baseType = TYPE_STRING;
    }
    else if (match(parser, TOKEN_BOOL))
    {
        baseType = TYPE_BOOL;
    }
    else if (match(parser, TOKEN_VOID))
    {
        baseType = TYPE_VOID;
    }
    else if (match(parser, TOKEN_IDENTIFIER))
    {
        // 自定义类型（如结构体、枚举等）
        baseType = TYPE_STRUCT; // 将标识符视为结构体类型
    }
    else
    {
        error(parser, "Expected type annotation.");
        type.kind = TYPE_SIMPLE;
        type.as.simple = TYPE_ANY;
        return type;
    }

    // 检查是否是数组类型
    if (match(parser, TOKEN_LBRACKET))
    {
        if (!match(parser, TOKEN_RBRACKET))
        {
            error(parser, "Expected ']' after '['.");
        }

        type.kind = TYPE_ARRAY;
        type.as.array.elementType = baseType;
        type.as.array.size = NULL; // 动态数组
    }
    else
    {
        type.kind = TYPE_SIMPLE;
        type.as.simple = baseType;
    }

    return type;
}
