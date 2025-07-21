#ifndef SPARROW_VALUE_H
#define SPARROW_VALUE_H

#include <stdbool.h>
#include "type_system.h"
// #include "environment.h"

typedef struct Environment Environment;
// 前向声明
typedef struct Value Value;
typedef struct Array Array;

// 结构体字段值前向声明
typedef struct StructFieldValue StructFieldValue;

// 定义值类型
typedef enum
{
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_NATIVE_FUNCTION,
    VAL_ARRAY,
    VAL_ENUM_VALUE,
    VAL_STRUCT
} ValueType;


// 枚举值结构
typedef struct
{
    char *enumName;    // 枚举类型名称
    char *memberName;  // 枚举成员名称
    int value;         // 枚举值
} EnumValue;

// 结构体值结构
typedef struct
{
    char *structName;           // 结构体类型名称
    StructFieldValue *fields;   // 字段值数组
    int fieldCount;             // 字段数量
} StructValue;

// 函数类型前向声明
typedef struct Function Function;
typedef struct NativeFunction NativeFunction;

// 数组结构
struct Array {
    Value *elements;
    int count;
    int capacity;
    BaseType elementType;
};

// 值结构
struct Value
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        char *string;
        Function *function;
        NativeFunction *nativeFunction;
        Array *array;
        EnumValue *enumValue;
        StructValue *structValue;
    } as;
};

// 结构体字段值定义（必须在 Value 定义之后）
struct StructFieldValue
{
    char *name;        // 字段名称
    Value *value;      // 字段值指针
};

// 函数类型
struct Function
{
    char *name;
    int arity;                  // 参数数量
    char **paramNames;          // 参数名
    TypeAnnotation *paramTypes; // 参数类型
    TypeAnnotation returnType;  // 返回类型
    struct Stmt *body;          // 函数体
    Environment *closure;       // 闭包环境
};

// 本地函数类型
struct NativeFunction
{
    char *name;
    int arity;
    Value (*function)(int argCount, Value *args);
};

// 值操作函数
Value createNull();
Value createBool(bool value);
Value createNumber(double value);
Value createString(const char *value);
Value createFunction(Function *function);
Value createNativeFunction(NativeFunction *function);
Value createEnumValue(const char *enumName, const char *memberName, int value);
Value createStruct(const char *structName, StructFieldValue *fields, int fieldCount);
void freeEnumValue(EnumValue *enumValue);
void freeStructValue(StructValue *structValue);

// 数组操作函数
Value createArray(BaseType elementType, int initialCapacity);
void arrayPush(Array *array, Value value);
Value arrayGet(Array *array, int index);
void arraySet(Array *array, int index, Value value);
int arrayLength(Array *array);

// 值比较和操作
bool valuesEqual(Value a, Value b);
void printValue(Value value);
Value copyValue(Value value);
void freeValue(Value value);

#endif // SPARROW_VALUE_H