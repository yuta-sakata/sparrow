#ifndef SPARROW_NATIVE_FUNCTIONS_H
#define SPARROW_NATIVE_FUNCTIONS_H

#include "interpreter.h"
#include "value.h"

// 注册所有原生函数
void registerAllNativeFunctions(Interpreter *interpreter);

// 创建原生函数对象
NativeFunction *createNativeFn(const char *name, int arity,
                               Value (*function)(int, Value *));

// 基础原生函数声明
Value printNative(int argCount, Value *args);
Value clockNative(int argCount, Value *args);
Value typeNative(int argCount, Value *args);

// // I/O 原生函数声明
// Value readLineNative(int argCount, Value* args);
// Value readFileNative(int argCount, Value* args);
// Value writeFileNative(int argCount, Value* args);

// // 数学原生函数声明
// Value sqrtNative(int argCount, Value* args);
// Value sinNative(int argCount, Value* args);
// Value cosNative(int argCount, Value* args);
// Value randomNative(int argCount, Value* args);

// // 字符串原生函数声明
// Value strLenNative(int argCount, Value* args);
// Value subStringNative(int argCount, Value* args);
// Value strConcatNative(int argCount, Value* args);

#endif // SPARROW_NATIVE_FUNCTIONS_H