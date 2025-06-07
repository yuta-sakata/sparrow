# 灵雀编程语言 (Sparrow)

灵雀是一款简洁、易用的编程语言，旨在为初学者提供友好的编程体验，同时也具备足够的表达能力满足更复杂的编程需求。目前该语言处于早期开发阶段，已实现基本功能框架。

## 简介

灵雀语言（Sparrow）是一种静态类型的脚本语言，语法设计参考了现代编程语言的优点，具有以下特性：

- 强类型系统，支持类型注解
- 函数是一等公民
- 简洁清晰的语法
- 支持变量声明与赋值
- 原生支持常见数据类型

## 当前实现状态

✅ **已实现功能：**
- 词法分析器 ([src/lexer.c](src/lexer.c))
- 语法分析器 ([src/parser.c](src/parser.c))
- 抽象语法树 ([src/ast.c](src/ast.c))
- 解释器执行引擎 ([src/interpreter.c](src/interpreter.c))
- 环境管理和变量作用域 ([src/environment.c](src/environment.c))
- 值系统和内存管理 ([src/value.c](src/value.c))
- 基本数据类型：数字、字符串、布尔值、null
- 算术运算：+、-、*、/、%
- 比较运算：<、<=、>、>=、==、!=
- 逻辑运算：&&、||、!
- 变量声明和赋值
- 函数定义和调用
- 控制流：if/else、while、for 循环
- 后缀运算符：++、--
- 内置函数：print、clock、type

## 项目结构

```
sparrow/
├── build/              # 编译中间文件
├── include/            # 头文件目录
│   ├── ast.h          # 抽象语法树定义
│   ├── lexer.h        # 词法分析器接口
│   ├── parser.h       # 语法分析器接口
│   ├── interpreter.h  # 解释器接口
│   ├── environment.h  # 环境管理接口
│   ├── value.h        # 值系统接口
│   ├── type_system.h  # 类型系统接口
│   ├── native_functions.h  # 内置函数接口
│   └── file_utils.h   # 文件工具接口
├── output/            # 编译产出
├── src/               # 源代码目录
│   ├── main.c         # 程序入口
│   ├── lexer.c        # 词法分析器实现
│   ├── parser.c       # 语法分析器实现
│   ├── ast.c          # 抽象语法树实现
│   ├── interpreter.c  # 解释器实现
│   ├── environment.c  # 环境管理实现
│   ├── value.c        # 值系统实现
│   ├── type_system.c  # 类型系统实现
│   ├── native_functions.c  # 内置函数实现
│   └── file_utils.c   # 文件工具实现
├── test.spw           # 示例代码
├── Makefile           # 构建脚本
└── README.md          # 本文档
```

## 安装与构建

### 依赖项

- GCC 编译器
- Make
- 数学库 (libm)

### 编译步骤

```bash
# 克隆仓库
git clone https://github.com/yuta-sakata/sparrow.git
cd sparrow

# 编译项目
make

# 运行示例
./output/sparrow test.spw
```

## Hello World 示例

目前，灵雀语言可以运行简单的 Hello World 程序：

```
// 创建一个 hello.spw 文件
function main():void {
    print("Hello, world!");
}
```

运行方式：

```bash
./output/sparrow hello.spw
```

## 语法示例

### Hello World

```sparrow
function main():void {
    print("Hello, World!");
}
```

### 变量声明

```sparrow
// 单个变量声明
var name:string = "Sparrow";
var age:int = 1;
var version:float = 1.0;
var isActive:bool = true;

// 多变量声明
var x, y, z:int = 0;
```

### 函数定义

```sparrow
// 带参数和返回值的函数
function add(var a:int, var b:int):int {
    return a + b;
}

// 无参数函数
function greet():void {
    print("Hello from Sparrow!");
}

// 调用函数
function main():void {
    var result:int = add(5, 3);
    print(result);  // 输出: 8
    greet();
}
```

### 控制流

```sparrow
function main():void {
    // if-else 语句
    var x:int = 10;
    if (x > 5) {
        print("x is greater than 5");
    } else {
        print("x is not greater than 5");
    }
    
    // while 循环
    var i:int = 0;
    while (i < 3) {
        print(i);
        i++;
    }
    
    // for 循环
    for (var j:int = 0; j < 3; j++) {
        print("Loop iteration: ");
        print(j);
    }
}
```

### 运算符

```sparrow
function main():void {
    // 算术运算
    var a:int = 10;
    var b:int = 3;
    print(a + b);  // 13
    print(a - b);  // 7
    print(a * b);  // 30
    print(a / b);  // 3.333...
    print(a % b);  // 1
    
    // 后缀运算符
    var counter:int = 5;
    print(counter++);  // 5 (先使用再递增)
    print(counter);    // 6
    print(++counter);  // 7 (先递增再使用)
    
    // 比较运算
    print(a > b);   // true
    print(a == b);  // false
    
    // 逻辑运算
    var condition1:bool = true;
    var condition2:bool = false;
    print(condition1 && condition2);  // false
    print(condition1 || condition2);  // true
    print(!condition1);               // false
}
```

## 基本数据类型

- `int`: 整数类型 (内部存储为 double)
- `float`: 浮点数类型 (内部存储为 double)
- `string`: 字符串类型
- `bool`: 布尔类型 (true/false)
- `void`: 空类型 (仅用于函数返回类型)
- `null`: 空值类型

## 内置函数

目前实现了以下内置函数：

- `print(value)`: 打印值到控制台
- `clock()`: 返回当前时间戳（秒）
- `type(value)`: 返回值的类型字符串

### 基本类型

- `int`: 整数类型
- `float`: 浮点数类型
- `string`: 字符串类型
- `bool`: 布尔类型
- `void`: 空类型

### 内置函数

目前实现了以下内置函数：
- `print()`: 打印值到控制台
- `clock()`: 返回当前时间戳
- `type()`: 返回值的类型

## 未来计划

灵雀语言仍处于早期开发阶段，计划添加的功能包括：

- 完整的错误处理机制
- 更多内置数据结构（数组、映射等）
- 模块和导入系统
- 面向对象编程支持
- 异常处理
- 标准库扩展

## 贡献

欢迎对灵雀语言进行贡献！如有任何问题或建议，请提交 issue 或 pull request。
