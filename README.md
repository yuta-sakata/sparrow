# 灵雀编程语言 (Sparrow)

灵雀是一款功能完整的解释型编程语言，支持静态类型系统、丰富的控制流、数组操作、函数式编程等现代编程语言特性。该语言语法简洁明了，专为学习和实验而设计。

## 语言特性

灵雀语言（Sparrow）是一种静态类型的解释型语言，具有以下核心特性：

- 🎯 **静态类型系统**：支持完整的类型注解和类型检查
- 🔧 **模块化架构**：采用模块化设计，易于扩展和维护
- 📊 **丰富的数据类型**：支持基本类型、数组、枚举等数据结构
- 🎛️ **完整的控制流**：if/else、while、do-while、for、switch语句
- 🚀 **函数式特性**：函数是一等公民，支持递归调用
- 🔒 **静态存储**：支持静态变量和常量
- 🛠️ **内置函数库**：提供丰富的内置函数和数组操作
- 🎨 **类型转换**：支持显式类型转换

## 完整功能列表

### ✅ 核心语言特性
- **词法分析器**：完整的词法分析，支持所有语言特性
- **语法分析器**：模块化解析器，支持复杂语法结构
- **抽象语法树**：完整的AST表示，支持所有语言结构
- **解释器引擎**：模块化解释器，高效执行代码
- **环境管理**：完整的作用域和变量管理
- **内存管理**：自动内存管理和垃圾回收

### ✅ 数据类型系统
- **基本类型**：`int`、`float`、`string`、`bool`、`void`、`null`
- **复合类型**：数组类型（如 `int[]`）
- **枚举类型**：支持有值和无值枚举
- **类型转换**：显式类型转换（如 `(float)intValue`）
- **类型检查**：运行时类型验证

### ✅ 变量和常量系统
- **变量声明**：单变量和多变量声明
- **常量声明**：单常量和多常量声明
- **静态存储**：静态变量和静态常量
- **作用域管理**：块级作用域和函数作用域

### ✅ 运算符系统
- **算术运算**：`+`、`-`、`*`、`/`、`%`
- **比较运算**：`<`、`<=`、`>`、`>=`、`==`、`!=`
- **逻辑运算**：`&&`、`||`、`!`
- **赋值运算**：`=`、数组元素赋值
- **一元运算**：`-`（负号）、`!`（逻辑非）
- **前缀运算**：`++var`、`--var`
- **后缀运算**：`var++`、`var--`

### ✅ 控制流结构
- **条件语句**：`if`、`else if`、`else`
- **循环语句**：`while`、`do-while`、`for`
- **跳转语句**：`break`、`return`
- **选择语句**：`switch`、`case`、`default`（支持fallthrough）

### ✅ 函数系统
- **函数定义**：支持参数和返回值类型
- **函数调用**：支持递归调用
- **静态函数**：静态函数声明和调用
- **参数传递**：值传递参数系统
- **返回值**：支持各种类型的返回值

### ✅ 数组操作
- **数组字面量**：`[1, 2, 3, 4, 5]`
- **数组访问**：`array[index]`
- **数组赋值**：`array[index] = value`
- **数组方法**：`length()`、`push()`、`pop()`、`slice()`

### ✅ 内置函数库
- **输入输出**：`print()`、`println()`、`input()`
- **系统函数**：`clock()`、`type()`
- **数组函数**：`length()`、`push()`、`pop()`、`popArray()`、`slice()`

## 项目架构

```
sparrow/
│
├── include/                 # 头文件目录
│   ├── ast.h               # 抽象语法树定义
│   ├── environment.h       # 环境管理接口
│   ├── file_utils.h        # 文件工具接口
│   ├── interpreter.h       # 解释器主接口
│   ├── lexer.h             # 词法分析器接口
│   ├── native_functions.h  # 内置函数接口
│   ├── parser.h            # 语法分析器主接口
│   ├── type_system.h       # 类型系统接口
│   ├── value.h             # 值系统接口
│   ├── interpreter/        # 解释器模块接口
│   │   ├── array_operations.h
│   │   ├── binary_operations.h
│   │   ├── cast_operations.h
│   │   ├── expression_evaluator.h
│   │   ├── function_calls.h
│   │   ├── interpreter_core.h
│   │   ├── statement_executor.h
│   │   └── unary_operations.h
│   └── parser/             # 解析器模块接口
│       ├── declaration_parser.h
│       ├── expression_parser.h
│       ├── parser_core.h
│       ├── statement_parser.h
│       └── type_parser.h
├── output/                 # 编译产出
│   └── sparrow            # 可执行文件
├── src/                   # 源代码目录
│   ├── main.c             # 程序入口点
│   ├── ast.c              # 抽象语法树实现
│   ├── environment.c      # 环境和作用域管理
│   ├── file_utils.c       # 文件读取工具
│   ├── lexer.c            # 词法分析器
│   ├── native_functions.c # 内置函数实现
│   ├── type_system.c      # 类型系统实现
│   ├── value.c            # 值系统和内存管理
│   ├── interpreter/       # 解释器模块
│   │   ├── interpreter_core.c      # 解释器核心
│   │   ├── expression_evaluator.c  # 表达式求值
│   │   ├── binary_operations.c     # 二元运算
│   │   ├── unary_operations.c      # 一元运算
│   │   ├── array_operations.c      # 数组操作
│   │   ├── function_calls.c        # 函数调用
│   │   ├── statement_executor.c    # 语句执行
│   │   └── cast_operations.c       # 类型转换
│   └── parser/            # 解析器模块
│       ├── parser_core.c          # 解析器核心
│       ├── declaration_parser.c   # 声明解析
│       ├── statement_parser.c     # 语句解析
│       ├── expression_parser.c    # 表达式解析
│       └── type_parser.c          # 类型解析
├── test.spw               # 测试文件
├── Makefile               # 构建配置
└── README.md              # 项目文档
```

## 快速开始

### 系统要求

- **编译器**：GCC (支持 C99 标准)
- **构建工具**：GNU Make
- **系统库**：数学库 (libm)
- **操作系统**：Linux、macOS、Windows (WSL)

### 安装与构建

```bash
# 克隆仓库
git clone https://github.com/yuta-sakata/sparrow.git
cd sparrow

# 编译项目
make

# 运行完整测试套件
make test
# 或者
./output/sparrow test.spw

# 清理构建文件
make clean
```

### Hello World

创建你的第一个灵雀程序：

```sparrow
// hello.spw
function main():void {
    println("Hello, Sparrow!");
    println("欢迎使用灵雀编程语言！");
}
```

运行程序：

```bash
./output/sparrow hello.spw
```

## 语法详解

### 数据类型

灵雀支持以下数据类型：

```sparrow
// 基本数据类型
var intValue:int = 42;
var floatValue:float = 3.14159;
var stringValue:string = "Hello, Sparrow!";
var boolValue:bool = true;

// 数组类型
var numbers:int[] = [1, 2, 3, 4, 5];
var names:string[] = ["Alice", "Bob", "Charlie"];

// 类型检查
println("intValue的类型:", type(intValue));
```

### 变量和常量声明

```sparrow
// 单个变量声明
var singleVar:int = 10;

// 多变量声明（共享类型和初始值）
var x, y, z:int = 5;

// 常量声明
const CONSTANT_VALUE:int = 99;

// 多常量声明
const A, B, C:string = "test";

// 静态变量和常量
static var globalCounter:int = 0;
static const PI:float = 3.14159;
static const MAX_SIZE, MIN_SIZE:int = 100;
```

### 枚举类型

```sparrow
// 有值枚举
enum Color {
    RED = 1,
    GREEN = 2,
    BLUE = 3
}

// 无值枚举（自动赋值）
enum Status {
    INACTIVE,    // 0
    ACTIVE,      // 1
    PENDING      // 2
}

// 使用枚举
var currentColor = Color.RED;
var currentStatus = Status.ACTIVE;

if (currentColor == Color.RED) {
    println("颜色是红色");
}
```

### 运算符

```sparrow
function testOperators():void {
    var a:int = 20, b:int = 6;
    
    // 算术运算
    println("加法:", a + b);      // 26
    println("减法:", a - b);      // 14
    println("乘法:", a * b);      // 120
    println("除法:", a / b);      // 3.333...
    println("取模:", a % b);      // 2
    
    // 前缀和后缀运算
    var c:int = 5;
    println("前缀递增:", ++c);    // 6
    println("后缀递增:", c++);    // 6 (然后 c 变为 7)
    
    // 比较运算
    println("大于:", a > b);      // true
    println("等于:", a == b);     // false
    
    // 逻辑运算
    var flag1:bool = true, flag2:bool = false;
    println("逻辑与:", flag1 && flag2);  // false
    println("逻辑或:", flag1 || flag2);  // true
    println("逻辑非:", !flag1);          // false
}
```

### 控制流

```sparrow
function testControlFlow():void {
    // if-else 语句
    var score:int = 85;
    if (score >= 90) {
        println("等级: A");
    } else if (score >= 80) {
        println("等级: B");
    } else {
        println("等级: C");
    }
    
    // while 循环
    var i:int = 1;
    while (i <= 3) {
        println("while循环:", i);
        i++;
    }
    
    // do-while 循环
    var j:int = 1;
    do {
        println("do-while循环:", j);
        j++;
    } while (j <= 2);
    
    // for 循环
    for (var k:int = 0; k < 3; k++) {
        println("for循环:", k);
    }
}
```

### Switch 语句

```sparrow
function testSwitch():void {
    var day:int = 3;
    
    switch (day) {
        case 1:
            println("星期一");
            break;
        case 2:
            println("星期二");
            break;
        case 3:
            println("星期三");
            break;
        default:
            println("其他日期");
            break;
    }
    
    // 支持 fallthrough
    var grade:string = "B";
    switch (grade) {
        case "A":
        case "B":
        case "C":
            println("及格");
            break;
        default:
            println("不及格");
            break;
    }
}
```

### 函数定义与调用

```sparrow
// 带参数和返回值的函数
function add(var a:int, var b:int):int {
    return a + b;
}

// 递归函数
function factorial(var n:int):int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// 静态函数
static function getInfo():string {
    return "这是静态函数";
}

// 主函数
function main():void {
    var sum:int = add(5, 3);
    var fact:int = factorial(5);
    println("5 + 3 =", sum);
    println("5! =", fact);
    println(getInfo());
}
```

### 数组操作

```sparrow
function testArrays():void {
    // 创建和初始化数组
    var numbers:int[] = [1, 2, 3, 4, 5];
    println("原始数组:", numbers);
    println("数组长度:", length(numbers));
    
    // 访问和修改元素
    println("第一个元素:", numbers[0]);
    numbers[2] = 99;
    println("修改后:", numbers);
    
    // 数组方法
    var newArray:int[] = push(numbers, 6);
    println("push后:", newArray);
    
    var lastElement = pop(newArray);
    var afterPop = popArray(newArray);
    println("弹出的元素:", lastElement);
    println("pop后数组:", afterPop);
    
    // 数组切片
    var sliced = slice(numbers, 1, 4);
    println("切片 [1:4]:", sliced);
}
```

### 类型转换

```sparrow
function testTypeCasting():void {
    var intVal:int = 42;
    var floatVal:float = 3.14;
    
    // 显式类型转换
    println("整数转浮点:", (float)intVal);    // 42.0
    println("浮点转整数:", (int)floatVal);    // 3
}
```

## 内置函数参考

### 输入输出函数

```sparrow
// 打印函数（不换行）
print("Hello", "World", 123);

// 打印函数（换行）
println("这会自动换行");
println("支持多个参数:", 42, true, 3.14);

// 用户输入
var name:string = input("请输入你的名字: ");
println("你好,", name);
```

### 系统函数

```sparrow
// 获取当前时间戳
var currentTime = clock();
println("当前时间戳:", currentTime);

// 获取值的类型
var value:int = 42;
println("值的类型:", type(value));  // 输出: int
```

### 数组函数

```sparrow
var arr:int[] = [1, 2, 3];

// 获取数组长度
var len = length(arr);
println("数组长度:", len);

// 添加元素（返回新数组）
var newArr = push(arr, 4);
println("添加元素后:", newArr);

// 弹出最后一个元素
var lastElement = pop(arr);        // 获取最后一个元素
var arrayAfterPop = popArray(arr); // 获取移除最后元素后的数组
println("弹出的元素:", lastElement);
println("剩余数组:", arrayAfterPop);

// 数组切片
var sliced = slice(arr, 0, 2);     // 从索引0到2（不包含2）
var slicedWithStep = slice(arr, 1); // 从索引1到末尾
```

## 完整示例程序

以下是一个展示灵雀语言主要特性的完整程序：

```sparrow
// 静态变量和常量
static var globalCounter:int = 0;
static const PI:float = 3.14159;

// 枚举定义
enum Color {
    RED = 1,
    GREEN = 2,
    BLUE = 3
}

// 递归函数示例
function fibonacci(var n:int):int {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// 数组处理函数
function processArray(var arr:int[]):void {
    println("处理数组:", arr);
    println("数组长度:", length(arr));
    
    // 计算数组元素和
    var sum:int = 0;
    for (var i:int = 0; i < length(arr); i++) {
        sum = sum + arr[i];
    }
    println("数组元素和:", sum);
}

// 主函数
function main():void {
    println("=== 灵雀语言演示程序 ===");
    
    // 基本数据类型
    var name:string = "Sparrow";
    var version:float = 1.0;
    var isStable:bool = true;
    
    println("语言名称:", name);
    println("版本:", version);
    println("稳定版本:", isStable);
    
    // 控制流演示
    if (isStable) {
        println("当前版本稳定");
    } else {
        println("当前版本为测试版");
    }
    
    // 循环和数组
    var numbers:int[] = [1, 2, 3, 4, 5];
    processArray(numbers);
    
    // 递归函数调用
    println("斐波那契数列前10项:");
    for (var i:int = 0; i < 10; i++) {
        print(fibonacci(i), " ");
    }
    println();
    
    // 枚举使用
    var currentColor = Color.RED;
    switch (currentColor) {
        case Color.RED:
            println("当前颜色: 红色");
            break;
        case Color.GREEN:
            println("当前颜色: 绿色");
            break;
        case Color.BLUE:
            println("当前颜色: 蓝色");
            break;
    }
    
    // 静态变量操作
    globalCounter = globalCounter + 1;
    println("全局计数器:", globalCounter);
    println("圆周率常量:", PI);
    
    println("程序执行完成!");
}
```

## 技术架构

### 模块化设计

灵雀语言采用高度模块化的架构设计：

- **词法分析器** (`lexer.c`): 将源代码转换为词法单元
- **语法分析器** (`parser/`): 模块化的递归下降解析器
  - `parser_core.c`: 解析器核心逻辑
  - `declaration_parser.c`: 声明语句解析
  - `statement_parser.c`: 语句解析
  - `expression_parser.c`: 表达式解析
  - `type_parser.c`: 类型解析
- **解释器** (`interpreter/`): 模块化的解释执行引擎
  - `interpreter_core.c`: 解释器核心
  - `expression_evaluator.c`: 表达式求值
  - `binary_operations.c`: 二元运算处理
  - `unary_operations.c`: 一元运算处理
  - `array_operations.c`: 数组操作
  - `function_calls.c`: 函数调用处理
  - `statement_executor.c`: 语句执行
  - `cast_operations.c`: 类型转换

### 内存管理

- **自动内存管理**: 自动分配和释放内存
- **值系统**: 统一的值表示和操作
- **环境管理**: 分层的作用域和变量管理
- **垃圾回收**: 及时释放不再使用的资源

### 类型系统

- **静态类型检查**: 编译时类型验证
- **类型推导**: 智能类型推导和检查
- **类型转换**: 安全的显式类型转换
- **类型兼容性**: 合理的类型兼容性规则

## 开发路线图

### 🎯 当前版本 (v1.0)
- ✅ 完整的核心语言特性
- ✅ 静态类型系统
- ✅ 函数式编程支持
- ✅ 数组操作
- ✅ 枚举和静态存储
- ✅ 完整的控制流
- ✅ 模块化架构

### 🚀 未来计划 (v1.1+)
- 📝 **错误处理增强**
  - 更详细的错误信息
  - 错误位置追踪
  - 编译时警告系统
  
- 📦 **数据结构扩展**
  - 哈希映射 (Map)
  - 集合 (Set)
  - 动态数组增强
  
- 🏗️ **面向对象特性**
  - 类和对象
  - 继承和多态
  - 接口定义
  
- 📚 **模块系统**
  - import/export 语句
  - 模块命名空间
  - 标准库分离
  
- ⚡ **性能优化**
  - 字节码生成
  - 虚拟机执行
  - 内存优化
  
- 🛠️ **开发工具**
  - 语法高亮
  - 调试器支持
  - IDE 集成
