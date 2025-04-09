# 灵雀编程语言 (Sparrow)

灵雀是一款简洁、易用的编程语言，旨在为初学者提供友好的编程体验，同时也具备足够的表达能力满足更复杂的编程需求。目前该语言处于早期开发阶段，已实现基本功能框架。

<h2 style="color:red">注意，以下大饼只实现了打印hello world且运行结束后崩溃</h2>

## 简介

灵雀语言（Sparrow）是一种静态类型的脚本语言，语法设计参考了现代编程语言的优点，具有以下特性：

- 强类型系统，支持类型注解
- 函数是一等公民
- 简洁清晰的语法
- 支持变量声明与赋值
- 原生支持常见数据类型

## 项目结构

```
sparrow/
├── build/          # 编译中间文件
├── include/        # 头文件
├── output/         # 编译产出
├── src/            # 源代码
│   ├── ast.c       # 抽象语法树实现
│   ├── lexer.c     # 词法分析器
│   ├── parser.c    # 语法分析器
│   ├── interpreter.c # 解释器
│   └── ...         # 其他实现文件
├── test.spw        # 示例代码
├── Makefile        # 构建脚本
└── README.md       # 本文档
```

## 安装与构建

### 依赖项

- GCC 编译器
- Make

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

## 基本语法

### 函数定义

```
function 函数名(参数1:类型, 参数2:类型):返回类型 {
    // 函数体
    return 返回值;
}
```

### 变量声明

```
var 变量名:类型 = 初始值;
int 变量名 = 初始值;  // 类型前缀声明方式
```

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
