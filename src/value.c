#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "environment.h" // 确保包含这个

// 创建空值
Value createNull()
{
    Value value;
    value.type = VAL_NULL;
    return value;
}

// 创建布尔值
Value createBool(bool value)
{
    Value val;
    val.type = VAL_BOOL;
    val.as.boolean = value;
    return val;
}

// 创建数值
Value createNumber(double value)
{
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = value;
    return val;
}

/**
 * 创建字符串类型的Value对象
 *
 * 该函数分配内存并复制输入的字符串，创建一个新的字符串类型Value。
 * 如果输入为NULL，则创建一个空字符串。如果内存分配失败，则返回NULL类型的Value。
 *
 * @param value 要复制的C字符串，可以为NULL
 * @return Value 包含复制字符串的Value对象，类型为VAL_STRING；
 *               如果内存分配失败则返回VAL_NULL类型的Value
 *
 * @note 调用者有责任通过适当的清理函数释放返回Value中分配的内存
 * @warning 如果内存分配失败，返回的Value类型将被设置为VAL_NULL
 */
Value createString(const char *value)
{
    Value val;
    val.type = VAL_STRING;

    // 处理NULL输入
    if (value == NULL)
    {
        val.as.string = malloc(1);
        if (val.as.string == NULL)
        {
            val.type = VAL_NULL;
            return val;
        }
        val.as.string[0] = '\0';
        return val;
    }

    // 计算字符串长度并检查有效性
    size_t len = strlen(value);

    val.as.string = (char *)malloc(len + 1);
    if (val.as.string == NULL)
    {
        val.type = VAL_NULL;
        return val;
    }

    strcpy(val.as.string, value);

    return val;
}

/**
 * 创建一个函数类型的值对象
 *
 * 此函数用于将一个Function指针包装成Value结构体，设置其类型为VAL_FUNCTION
 * 并将函数指针存储在联合体的function字段中。
 *
 * @param function 指向要包装的Function结构体的指针
 * @return 返回一个类型为VAL_FUNCTION的Value结构体，其中包含传入的函数指针
 */
Value createFunction(Function *function)
{
    Value val;
    val.type = VAL_FUNCTION;
    val.as.function = function;
    return val;
}

// 创建原生函数值
Value createNativeFunction(NativeFunction *function)
{
    Value val;
    val.type = VAL_NATIVE_FUNCTION;
    val.as.nativeFunction = function;
    return val;
}

// 值比较
bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type)
        return false;

    switch (a.type)
    {
    case VAL_NULL:
        return true;
    case VAL_BOOL:
        return a.as.boolean == b.as.boolean;
    case VAL_NUMBER:
        return a.as.number == b.as.number;
    case VAL_STRING:
        return strcmp(a.as.string, b.as.string) == 0;
    case VAL_FUNCTION:
        return a.as.function == b.as.function;
    case VAL_NATIVE_FUNCTION:
        return a.as.nativeFunction == b.as.nativeFunction;
    case VAL_ARRAY:
        if (a.as.array == NULL || b.as.array == NULL)
        {
            return a.as.array == b.as.array;
        }
        
        // 比较数组长度
        if (a.as.array->count != b.as.array->count)
        {
            return false;
        }
        
        // 比较每个元素
        for (int i = 0; i < a.as.array->count; i++)
        {
            if (!valuesEqual(a.as.array->elements[i], b.as.array->elements[i]))
            {
                return false;
            }
        }
        return true;
    case VAL_ENUM_VALUE:
        if (a.as.enumValue == NULL || b.as.enumValue == NULL)
        {
            return a.as.enumValue == b.as.enumValue;
        }
        return a.as.enumValue->value == b.as.enumValue->value &&
               strcmp(a.as.enumValue->enumName, b.as.enumValue->enumName) == 0;
    case VAL_STRUCT:
        if (a.as.structValue == NULL || b.as.structValue == NULL)
        {
            return a.as.structValue == b.as.structValue;
        }
        
        // 比较结构体类型名称
        if (a.as.structValue->structName == NULL || b.as.structValue->structName == NULL)
        {
            if (a.as.structValue->structName != b.as.structValue->structName)
                return false;
        }
        else if (strcmp(a.as.structValue->structName, b.as.structValue->structName) != 0)
        {
            return false;
        }
        
        // 比较字段数量
        if (a.as.structValue->fieldCount != b.as.structValue->fieldCount)
        {
            return false;
        }
        
        // 比较每个字段
        for (int i = 0; i < a.as.structValue->fieldCount; i++)
        {
            // 比较字段名称
            if (a.as.structValue->fields[i].name == NULL || b.as.structValue->fields[i].name == NULL)
            {
                if (a.as.structValue->fields[i].name != b.as.structValue->fields[i].name)
                    return false;
            }
            else if (strcmp(a.as.structValue->fields[i].name, b.as.structValue->fields[i].name) != 0)
            {
                return false;
            }
            
            // 比较字段值
            Value *aValue = a.as.structValue->fields[i].value;
            Value *bValue = b.as.structValue->fields[i].value;
            
            if (aValue == NULL || bValue == NULL)
            {
                if (aValue != bValue)
                    return false;
            }
            else if (!valuesEqual(*aValue, *bValue))
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

/**
 * 打印值的内容到标准输出
 *
 * 根据值的类型格式化并打印相应的内容：
 * - VAL_NULL: 打印 "null"
 * - VAL_BOOL: 打印 "true" 或 "false"
 * - VAL_NUMBER: 如果是整数则打印整数格式，否则打印浮点数格式
 * - VAL_STRING: 打印字符串内容，如果为空则打印 "(null string)"
 * - VAL_FUNCTION: 打印函数信息，格式为 "[Function: 函数名]" 或 "[Function: anonymous]"
 * - VAL_NATIVE_FUNCTION: 打印原生函数信息，格式为 "[Native Function: 函数名]" 或 "[Native Function: anonymous]"
 * - VAL_ARRAY: 打印数组内容，格式为 "[元素1, 元素2, ...]"，递归打印每个元素
 * - 其他类型: 打印 "(unknown value type)"
 *
 * @param value 要打印的值对象
 *
 * @note 对于数组类型，函数会递归调用自身来打印每个元素
 * @note 包含空指针检查以防止访问无效内存
 * @note 对于数值类型，会自动判断是否为整数并选择合适的打印格式
 */
void printValue(Value value)
{
    switch (value.type)
    {
    case VAL_NULL:
        printf("null");
        break;
    case VAL_BOOL:
        printf(value.as.boolean ? "true" : "false");
        break;
    case VAL_NUMBER:
        // 检查是否为整数
        if (value.as.number == (int)value.as.number)
        {
            printf("%d", (int)value.as.number);
        }
        else
        {
            printf("%g", value.as.number);
        }
        break;
    case VAL_STRING:
        if (value.as.string != NULL)
        {
            printf("%s", value.as.string);
        }
        else
        {
            printf("(null string)");
        }
        break;
    case VAL_FUNCTION:
        if (value.as.function != NULL && value.as.function->name != NULL)
        {
            printf("[Function: %s]", value.as.function->name);
        }
        else
        {
            printf("[Function: anonymous]");
        }
        break;
    case VAL_NATIVE_FUNCTION:
        if (value.as.nativeFunction != NULL && value.as.nativeFunction->name != NULL)
        {
            printf("[Native Function: %s]", value.as.nativeFunction->name);
        }
        else
        {
            printf("[Native Function: anonymous]");
        }
        break;
    case VAL_ARRAY:
        if (value.as.array == NULL)
        {
            printf("[]");
            break;
        }

        printf("[");
        for (int i = 0; i < value.as.array->count; i++)
        {
            // 递归调用前添加安全检查
            if (i < value.as.array->capacity && value.as.array->elements != NULL)
            {
                printValue(value.as.array->elements[i]);
            }
            else
            {
                printf("(invalid element)");
            }

            if (i < value.as.array->count - 1)
            {
                printf(", ");
            }
        }
        printf("]");
        break;
    case VAL_ENUM_VALUE:
        if (value.as.enumValue != NULL)
        {
            if (value.as.enumValue->enumName != NULL && value.as.enumValue->memberName != NULL)
            {
                printf("%s::%s", value.as.enumValue->enumName, value.as.enumValue->memberName);
            }
            else
            {
                printf("(invalid enum value)");
            }
        }
        else
        {
            printf("(null enum value)");
        }
        break;
    case VAL_STRUCT:
        if (value.as.structValue != NULL)
        {
            if (value.as.structValue->structName != NULL)
            {
                printf("%s{", value.as.structValue->structName);
            }
            else
            {
                printf("{");
            }
            
            for (int i = 0; i < value.as.structValue->fieldCount; i++)
            {
                if (value.as.structValue->fields[i].name != NULL)
                {
                    printf("%s: ", value.as.structValue->fields[i].name);
                    if (value.as.structValue->fields[i].value != NULL)
                    {
                        printValue(*value.as.structValue->fields[i].value);
                    }
                    else
                    {
                        printf("null");
                    }
                }
                else
                {
                    printf("(invalid field)");
                }
                
                if (i < value.as.structValue->fieldCount - 1)
                {
                    printf(", ");
                }
            }
            printf("}");
        }
        else
        {
            printf("(null struct value)");
        }
        break;
    default:
        printf("(unknown value type)");
        break;
    }
}

Value createEnumValue(const char *enumName, const char *memberName, int value)
{
    Value val;
    val.type = VAL_ENUM_VALUE;

    EnumValue *enumValue = (EnumValue *)malloc(sizeof(EnumValue));
    if (enumValue == NULL)
    {
        return createNull();
    }

    // 复制枚举类型名称
    if (enumName != NULL)
    {
        size_t enumNameLen = strlen(enumName);
        enumValue->enumName = (char *)malloc(enumNameLen + 1);
        if (enumValue->enumName == NULL)
        {
            free(enumValue);
            return createNull();
        }
        strcpy(enumValue->enumName, enumName);
    }
    else
    {
        enumValue->enumName = NULL;
    }

    // 复制枚举成员名称
    if (memberName != NULL)
    {
        size_t memberNameLen = strlen(memberName);
        enumValue->memberName = (char *)malloc(memberNameLen + 1);
        if (enumValue->memberName == NULL)
        {
            free(enumValue->enumName);
            free(enumValue);
            return createNull();
        }
        strcpy(enumValue->memberName, memberName);
    }
    else
    {
        enumValue->memberName = NULL;
    }

    enumValue->value = value;
    val.as.enumValue = enumValue;

    return val;
}

/**
 * 创建结构体值
 */
Value createStruct(const char *structName, StructFieldValue *fields, int fieldCount)
{
    Value val;
    val.type = VAL_STRUCT;

    StructValue *structValue = (StructValue *)malloc(sizeof(StructValue));
    if (structValue == NULL)
    {
        return createNull();
    }

    // 复制结构体类型名称
    if (structName != NULL)
    {
        size_t nameLen = strlen(structName);
        structValue->structName = (char *)malloc(nameLen + 1);
        if (structValue->structName == NULL)
        {
            free(structValue);
            return createNull();
        }
        strcpy(structValue->structName, structName);
    }
    else
    {
        structValue->structName = NULL;
    }

    // 复制字段
    if (fieldCount > 0 && fields != NULL)
    {
        structValue->fields = (StructFieldValue *)malloc(sizeof(StructFieldValue) * fieldCount);
        if (structValue->fields == NULL)
        {
            free(structValue->structName);
            free(structValue);
            return createNull();
        }

        for (int i = 0; i < fieldCount; i++)
        {
            // 复制字段名称
            if (fields[i].name != NULL)
            {
                size_t fieldNameLen = strlen(fields[i].name);
                structValue->fields[i].name = (char *)malloc(fieldNameLen + 1);
                if (structValue->fields[i].name == NULL)
                {
                    // 清理已分配的内存
                    for (int j = 0; j < i; j++)
                    {
                        free(structValue->fields[j].name);
                        if (structValue->fields[j].value != NULL)
                        {
                            freeValue(*structValue->fields[j].value);
                            free(structValue->fields[j].value);
                        }
                    }
                    free(structValue->fields);
                    free(structValue->structName);
                    free(structValue);
                    return createNull();
                }
                strcpy(structValue->fields[i].name, fields[i].name);
            }
            else
            {
                structValue->fields[i].name = NULL;
            }

            // 复制字段值
            if (fields[i].value != NULL)
            {
                structValue->fields[i].value = (Value *)malloc(sizeof(Value));
                if (structValue->fields[i].value == NULL)
                {
                    // 清理已分配的内存
                    for (int j = 0; j < i; j++)
                    {
                        free(structValue->fields[j].name);
                        if (structValue->fields[j].value != NULL)
                        {
                            freeValue(*structValue->fields[j].value);
                            free(structValue->fields[j].value);
                        }
                    }
                    free(structValue->fields[i].name);
                    free(structValue->fields);
                    free(structValue->structName);
                    free(structValue);
                    return createNull();
                }
                *structValue->fields[i].value = copyValue(*fields[i].value);
            }
            else
            {
                structValue->fields[i].value = NULL;
            }
        }
    }
    else
    {
        structValue->fields = NULL;
    }

    structValue->fieldCount = fieldCount;
    val.as.structValue = structValue;

    return val;
}

void freeEnumValue(EnumValue *enumValue)
{
    if (enumValue == NULL)
        return;

    if (enumValue->enumName != NULL)
    {
        free(enumValue->enumName);
    }

    if (enumValue->memberName != NULL)
    {
        free(enumValue->memberName);
    }

    free(enumValue);
}

/**
 * 释放结构体值内存
 */
void freeStructValue(StructValue *structValue)
{
    if (structValue == NULL)
        return;

    if (structValue->structName != NULL)
    {
        free(structValue->structName);
    }

    if (structValue->fields != NULL)
    {
        for (int i = 0; i < structValue->fieldCount; i++)
        {
            if (structValue->fields[i].name != NULL)
            {
                free(structValue->fields[i].name);
            }
            if (structValue->fields[i].value != NULL)
            {
                freeValue(*structValue->fields[i].value);
                free(structValue->fields[i].value);
            }
        }
        free(structValue->fields);
    }

    free(structValue);
}

/**
 * 深度复制一个Value值
 *
 * 该函数根据Value的类型执行相应的复制操作：
 * - VAL_STRING: 创建新的字符串副本，处理NULL指针情况
 * - VAL_FUNCTION: 深度复制函数对象，包括函数名、参数名、参数类型等，
 *   但body和closure保持引用（浅拷贝）
 * - VAL_NATIVE_FUNCTION: 深度复制原生函数对象，包括函数指针、参数数量和函数名
 * - VAL_ARRAY: 递归深度复制数组及其所有元素
 * - 其他类型: 直接返回原值（简单值类型）
 *
 * @param value 要复制的Value值
 * @return Value 复制后的新Value值，如果复制失败则返回NULL值
 *
 * @note 函数会处理内存分配失败的情况，并在失败时清理已分配的资源
 * @note 对于函数类型，body和closure字段不进行深度复制以避免循环引用
 * @warning 调用者负责释放返回值占用的内存
 */
Value copyValue(Value value)
{
    switch (value.type)
    {

    // 处理字符串类型
    case VAL_STRING:
        if (value.as.string != NULL)
        {
            return createString(value.as.string); // 创建新的字符串副本
        }
        else
        {
            return createString(""); // 防止NULL指针
        }

    // 处理函数类型
    case VAL_FUNCTION:
    {
        // 深度复制函数对象
        Function *original = value.as.function;
        if (original == NULL)
        {
            return createNull();
        }

        // 分配新的函数结构
        Function *newFunction = (Function *)malloc(sizeof(Function));
        if (newFunction == NULL)
        {
            printf("ERROR: Failed to allocate memory for function copy\n");
            return createNull();
        }

        // 复制基本类型字段
        newFunction->arity = original->arity;
        newFunction->returnType = original->returnType;

        // 复制函数名
        if (original->name != NULL)
        {
            size_t nameLen = strlen(original->name);
            newFunction->name = (char *)malloc(nameLen + 1);
            if (newFunction->name == NULL)
            {
                free(newFunction);
                printf("ERROR: Failed to allocate memory for function name\n");
                return createNull();
            }
            strcpy(newFunction->name, original->name);
        }
        else
        {
            newFunction->name = NULL;
        }

        // 复制参数名
        newFunction->paramNames = NULL;
        if (original->arity > 0 && original->paramNames != NULL)
        {
            newFunction->paramNames = (char **)malloc(sizeof(char *) * original->arity);
            if (newFunction->paramNames == NULL)
            {
                free(newFunction->name);
                free(newFunction);
                printf("ERROR: Failed to allocate memory for param names\n");
                return createNull();
            }

            // 初始化为NULL，避免释放未分配内存的错误
            for (int i = 0; i < original->arity; i++)
            {
                newFunction->paramNames[i] = NULL;
            }

            // 复制每个参数名
            for (int i = 0; i < original->arity; i++)
            {
                if (original->paramNames[i] != NULL)
                {
                    size_t paramLen = strlen(original->paramNames[i]);
                    newFunction->paramNames[i] = (char *)malloc(paramLen + 1);
                    if (newFunction->paramNames[i] == NULL)
                    {
                        // 清理已分配资源
                        for (int j = 0; j < i; j++)
                        {
                            free(newFunction->paramNames[j]);
                        }
                        free(newFunction->paramNames);
                        free(newFunction->name);
                        free(newFunction);
                        printf("ERROR: Failed to allocate memory for param name\n");
                        return createNull();
                    }
                    strcpy(newFunction->paramNames[i], original->paramNames[i]);
                }
            }
        }

        // 复制参数类型（如果存在）
        newFunction->paramTypes = NULL;
        if (original->arity > 0 && original->paramTypes != NULL)
        {
            newFunction->paramTypes = (TypeAnnotation *)malloc(sizeof(TypeAnnotation) * original->arity);
            if (newFunction->paramTypes == NULL)
            {
                // 清理已分配资源
                for (int i = 0; i < original->arity; i++)
                {
                    if (newFunction->paramNames[i] != NULL)
                    {
                        free(newFunction->paramNames[i]);
                    }
                }
                free(newFunction->paramNames);
                free(newFunction->name);
                free(newFunction);
                printf("ERROR: Failed to allocate memory for param types\n");
                return createNull();
            }

            // 复制参数类型
            memcpy(newFunction->paramTypes, original->paramTypes, sizeof(TypeAnnotation) * original->arity);
        }

        // 对于body和closure保持引用，不进行深度复制
        newFunction->body = original->body;
        newFunction->closure = original->closure;

        // 创建新的函数值并返回
        Value newValue;
        newValue.type = VAL_FUNCTION;
        newValue.as.function = newFunction;
        return newValue;
    }

    // 处理原生函数类型
    case VAL_NATIVE_FUNCTION:
    {
        // 深度复制原生函数对象
        NativeFunction *original = value.as.nativeFunction;
        if (original == NULL)
        {
            return createNull();
        }

        // 分配新的原生函数结构
        NativeFunction *newNativeFunction = (NativeFunction *)malloc(sizeof(NativeFunction));
        if (newNativeFunction == NULL)
        {
            printf("ERROR: Failed to allocate memory for native function copy\n");
            return createNull();
        }

        // 复制函数指针和参数数量
        newNativeFunction->function = original->function;
        newNativeFunction->arity = original->arity;
        newNativeFunction->name = NULL;

        // 复制函数名
        if (original->name != NULL)
        {
            size_t nameLen = strlen(original->name);
            newNativeFunction->name = (char *)malloc(nameLen + 1);
            if (newNativeFunction->name == NULL)
            {
                free(newNativeFunction);
                printf("ERROR: Failed to allocate memory for native function name\n");
                return createNull();
            }
            strcpy(newNativeFunction->name, original->name);
        }

        // 创建新的原生函数值并返回
        Value newValue;
        newValue.type = VAL_NATIVE_FUNCTION;
        newValue.as.nativeFunction = newNativeFunction;
        return newValue;
    }
    case VAL_ARRAY:
        if (value.as.array == NULL)
        {
            return createNull();
        }

        // 添加安全检查
        Array *originalArray = value.as.array;

        // 检查数组结构的完整性
        if (originalArray->elements == NULL && originalArray->count > 0)
        {
            printf("WARNING: Array with NULL elements but non-zero count\n");
            return createNull();
        }

        if (originalArray->count < 0 || originalArray->capacity < 0)
        {
            printf("WARNING: Array with invalid count or capacity\n");
            return createNull();
        }

        // 添加额外的安全检查：确保 count 不超过 capacity
        if (originalArray->count > originalArray->capacity)
        {
            printf("WARNING: Array count (%d) exceeds capacity (%d)\n",
                   originalArray->count, originalArray->capacity);
            // 修复：不要直接返回 null，而是尝试修复这个问题
            originalArray->count = originalArray->capacity;
        }

        Value newArrayValue = createArray(originalArray->elementType, originalArray->capacity);

        if (newArrayValue.type == VAL_NULL)
        {
            return createNull();
        }

        // 复制所有元素
        Array *newArray = newArrayValue.as.array;
        // 确保不会复制超过实际存在的元素
        int elementsToCopy = originalArray->count;
        if (elementsToCopy > originalArray->capacity)
        {
            elementsToCopy = originalArray->capacity;
        }

        for (int i = 0; i < elementsToCopy; i++)
        {
            // 确保不会超出边界，并且原数组的元素指针有效
            if (i < originalArray->capacity && i < newArray->capacity &&
                originalArray->elements != NULL)
            {
                // 添加额外的安全检查来避免访问未初始化的内存
                Value elementToCopy = originalArray->elements[i];

                // 检查元素是否有效
                if (elementToCopy.type == VAL_STRING && elementToCopy.as.string == NULL)
                {
                    newArray->elements[i] = createNull();
                }
                else if (elementToCopy.type == VAL_ARRAY && elementToCopy.as.array == NULL)
                {
                    newArray->elements[i] = createNull();
                }
                else if (elementToCopy.type == VAL_FUNCTION && elementToCopy.as.function == NULL)
                {
                    newArray->elements[i] = createNull();
                }
                else if (elementToCopy.type == VAL_NATIVE_FUNCTION && elementToCopy.as.nativeFunction == NULL)
                {
                    newArray->elements[i] = createNull();
                }
                else
                {
                    newArray->elements[i] = copyValue(elementToCopy);
                }
                newArray->count++;
            }
            else
            {
                break; // 安全退出循环
            }
        }
        return newArrayValue;

    case VAL_ENUM_VALUE:
        if (value.as.enumValue != NULL)
        {
            return createEnumValue(
                value.as.enumValue->enumName,
                value.as.enumValue->memberName,
                value.as.enumValue->value);
        }
        else
        {
            return createNull();
        }
    case VAL_STRUCT:
        if (value.as.structValue != NULL)
        {
            // 复制字段数组
            StructFieldValue *fieldsCopy = NULL;
            if (value.as.structValue->fieldCount > 0)
            {
                fieldsCopy = (StructFieldValue *)malloc(sizeof(StructFieldValue) * value.as.structValue->fieldCount);
                if (fieldsCopy == NULL)
                {
                    return createNull();
                }
                
                for (int i = 0; i < value.as.structValue->fieldCount; i++)
                {
                    // 复制字段名称
                    if (value.as.structValue->fields[i].name != NULL)
                    {
                        size_t nameLen = strlen(value.as.structValue->fields[i].name);
                        fieldsCopy[i].name = (char *)malloc(nameLen + 1);
                        if (fieldsCopy[i].name == NULL)
                        {
                            // 清理已分配的内存
                            for (int j = 0; j < i; j++)
                            {
                                free(fieldsCopy[j].name);
                                if (fieldsCopy[j].value != NULL)
                                {
                                    freeValue(*fieldsCopy[j].value);
                                    free(fieldsCopy[j].value);
                                }
                            }
                            free(fieldsCopy);
                            return createNull();
                        }
                        strcpy(fieldsCopy[i].name, value.as.structValue->fields[i].name);
                    }
                    else
                    {
                        fieldsCopy[i].name = NULL;
                    }
                    
                    // 递归复制字段值
                    if (value.as.structValue->fields[i].value != NULL)
                    {
                        fieldsCopy[i].value = (Value *)malloc(sizeof(Value));
                        if (fieldsCopy[i].value == NULL)
                        {
                            // 清理已分配的内存
                            for (int j = 0; j < i; j++)
                            {
                                free(fieldsCopy[j].name);
                                if (fieldsCopy[j].value != NULL)
                                {
                                    freeValue(*fieldsCopy[j].value);
                                    free(fieldsCopy[j].value);
                                }
                            }
                            free(fieldsCopy[i].name);
                            free(fieldsCopy);
                            return createNull();
                        }
                        *fieldsCopy[i].value = copyValue(*value.as.structValue->fields[i].value);
                    }
                    else
                    {
                        fieldsCopy[i].value = NULL;
                    }
                }
            }
            
            return createStruct(value.as.structValue->structName, fieldsCopy, value.as.structValue->fieldCount);
        }
        else
        {
            return createNull();
        }
    default:
        // 对于简单值类型，直接复制
        return value;
    }
}

// 释放值
void freeValue(Value value)
{
    switch (value.type)
    {
    case VAL_STRING:
        if (value.as.string != NULL)
        {
            free(value.as.string);
        }
        break;

    case VAL_FUNCTION:
        // 释放函数资源
        if (value.as.function != NULL)
        {
            if (value.as.function->name != NULL)
            {
                free(value.as.function->name);
            }

            // 释放参数名
            if (value.as.function->paramNames != NULL)
            {
                for (int i = 0; i < value.as.function->arity; i++)
                {
                    if (value.as.function->paramNames[i] != NULL)
                    {
                        free(value.as.function->paramNames[i]);
                    }
                }
                free(value.as.function->paramNames);
            }

            // 注意：不释放 body 和 closure，它们由其他部分管理
            free(value.as.function);
        }
        break;

    // 释放原生函数资源
    case VAL_NATIVE_FUNCTION:
        // 原生函数需要释放结构体，但不释放函数指针
        if (value.as.nativeFunction != NULL)
        {

            // 安全释放原生函数名称
            if (value.as.nativeFunction->name != NULL)
            {
                free(value.as.nativeFunction->name);
                value.as.nativeFunction->name = NULL; // 防止悬空指针
            }

            // 释放原生函数结构体本身
            free(value.as.nativeFunction);
        }
        break;

    // 释放数组资源
    case VAL_ARRAY:
        if (value.as.array != NULL)
        {
            Array *array = value.as.array;

            // 添加双重释放检查
            if (array->count == -1 && array->capacity == -1)
            {
                // 数组已经被释放过了
                return;
            }

            // 检查数组是否已经被部分释放
            if (array->elements != NULL)
            {
                // 验证数组结构的完整性
                if (array->count >= 0 && array->capacity >= 0 && array->count <= array->capacity)
                {
                    for (int i = 0; i < array->count; i++)
                    {
                        if (i < array->capacity)
                        {
                            freeValue(array->elements[i]);
                        }
                    }
                }
                free(array->elements);
                array->elements = NULL; // 防止悬空指针
            }

            // 标记数组为无效状态
            array->count = -1;
            array->capacity = -1;

            free(array);
        }
        break;
    case VAL_ENUM_VALUE:
        if (value.as.enumValue != NULL)
        {
            freeEnumValue(value.as.enumValue);
        }
        break;
    case VAL_STRUCT:
        if (value.as.structValue != NULL)
        {
            freeStructValue(value.as.structValue);
        }
        break;
    default:
        // 其他类型不需要释放
        break;
    }
}

/**
 * 创建一个新的数组值
 *
 * 此函数分配并初始化一个新的数组结构，设置指定的元素类型和初始容量。
 * 如果内存分配失败，函数将返回一个类型为 VAL_NULL 的值。
 *
 * @param elementType 数组中元素的基础类型
 * @param initialCapacity 数组的初始容量。如果小于等于0，则默认使用8作为初始容量
 * @return Value 包含新创建数组的值结构体，如果内存分配失败则返回 VAL_NULL 类型的值
 *
 * @note 调用者负责在不再需要时释放返回的数组内存
 * @note 如果 initialCapacity <= 0，函数会自动设置容量为8
 */
Value createArray(BaseType elementType, int initialCapacity)
{
    Value val;
    val.type = VAL_ARRAY;

    Array *array = (Array *)malloc(sizeof(Array));
    if (array == NULL)
    {
        val.type = VAL_NULL;
        return val;
    }

    array->capacity = initialCapacity > 0 ? initialCapacity : 8;
    array->count = 0;
    array->elementType = elementType;
    array->elements = (Value *)calloc(array->capacity, sizeof(Value)); // 使用 calloc 初始化为0

    if (array->elements == NULL)
    {
        free(array);
        val.type = VAL_NULL;
        return val;
    }

    // 显式初始化所有元素为 NULL
    for (int i = 0; i < array->capacity; i++)
    {
        array->elements[i] = createNull();
    }

    val.as.array = array;
    return val;
}

/**
 * 向动态数组末尾添加一个元素
 *
 * 该函数将指定的值添加到数组的末尾。如果数组容量不足，会自动扩容。
 * 初始容量为8，之后每次扩容为当前容量的2倍。
 *
 * @param array 指向要操作的数组结构的指针，不能为NULL
 * @param value 要添加到数组中的值
 *
 * @note 如果传入NULL数组指针，函数会打印错误信息并返回
 * @note 如果内存重新分配失败，函数会打印错误信息并返回，原数组保持不变
 * @note 成功添加元素后，数组的count会自动增加1
 */
void arrayPush(Array *array, Value value)
{
    if (array == NULL)
    {
        printf("ERROR: NULL array passed to arrayPush\n");
        return;
    }

    if (array->count >= array->capacity)
    {
        int newCapacity = array->capacity == 0 ? 8 : array->capacity * 2;
        Value *newElements = (Value *)realloc(array->elements, sizeof(Value) * newCapacity);

        if (newElements == NULL)
        {
            printf("ERROR: Failed to reallocate memory for array\n");
            return;
        }

        array->elements = newElements;

        // 初始化新分配的内存
        for (int i = array->capacity; i < newCapacity; i++)
        {
            array->elements[i] = createNull();
        }

        array->capacity = newCapacity;
    }

    // 使用 copyValue 而不是直接赋值
    array->elements[array->count] = copyValue(value);
    array->count++;
}

Value arrayGet(Array *array, int index)
{
    if (array == NULL || index < 0 || index >= array->count)
    {
        return createNull();
    }

    return copyValue(array->elements[index]);
}

void arraySet(Array *array, int index, Value value)
{
    if (array == NULL || index < 0)
        return;

    // 如果索引超出当前范围，扩展数组
    if (index >= array->capacity)
    {
        int newCapacity = index + 1;
        if (newCapacity < array->capacity * 2)
            newCapacity = array->capacity * 2;

        Value *newElements = (Value *)realloc(array->elements, sizeof(Value) * newCapacity);
        if (newElements == NULL)
        {
            printf("ERROR: Failed to reallocate array memory\n");
            return;
        }
        array->elements = newElements;

        // 初始化新分配的内存
        for (int i = array->capacity; i < newCapacity; i++)
        {
            array->elements[i] = createNull();
        }

        array->capacity = newCapacity;
    }

    // 填充中间的空位
    while (array->count <= index)
    {
        if (array->count < array->capacity)
        {
            array->elements[array->count] = createNull();
            array->count++;
        }
        else
        {
            // 这里应该不会发生，因为上面已经扩展了容量
            printf("ERROR: Array count would exceed capacity\n");
            return;
        }
    }

    // 修复：释放旧值并设置新值
    if (index < array->count && index < array->capacity)
    {
        freeValue(array->elements[index]);
    }

    array->elements[index] = copyValue(value);
}

int arrayLength(Array *array)
{
    return array ? array->count : 0;
}
