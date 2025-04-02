#ifndef SPARROW_VALUE_H
#define SPARROW_VALUE_H

#include <stdbool.h>
#include "type_system.h"
// #include "environment.h"

typedef struct Environment Environment;

// 定义值类型
typedef enum
{
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_NATIVE_FUNCTION
} ValueType;

// 函数类型前向声明
typedef struct Function Function;
typedef struct NativeFunction NativeFunction;

// 值结构
typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        char *string;
        Function *function;
        NativeFunction *nativeFunction;
    } as;
} Value;

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

// 值比较和操作
bool valuesEqual(Value a, Value b);
void printValue(Value value);
Value copyValue(Value value);
void freeValue(Value value);

bool isValueCompatibleWithType(Value value, TypeAnnotation type);

#endif // SPARROW_VALUE_H