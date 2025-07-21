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
Value printNative(int argCount, Value *args);   // 打印
Value clockNative(int argCount, Value *args);   // 获取当前时间
Value typeNative(int argCount, Value *args);    // 获取值类型
Value inputNative(int argCount, Value *args);   // 获取用户输入
Value printlnNative(int argCount, Value *args); // 打印并换行

// 数组相关的原生函数
Value lengthNative(int argCount, Value *args); // 获取数组长度
Value pushNative(int argCount, Value *args);   // 向动态数组添加元素
Value popNative(int argCount, Value *args);    // 从动态数组获取最后元素
Value popArrayNative(int argCount, Value *args); // 从动态数组移除最后元素，返回新数组
Value sliceNative(int argCount, Value *args);  // 数组切片

#endif // SPARROW_NATIVE_FUNCTIONS_H