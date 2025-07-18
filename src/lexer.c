#include "lexer.h"

// 关键字查找结构
typedef struct
{
    const char *keyword;
    int length;
    TokenType type;
} Keyword;

// 关键字表
static Keyword keywords[] = {
    {"if", 2, TOKEN_IF},
    {"else", 4, TOKEN_ELSE},
    {"in", 2, TOKEN_IN},
    {"while", 5, TOKEN_WHILE},
    {"for", 3, TOKEN_FOR},
    {"do", 2, TOKEN_DO},
    {"return", 6, TOKEN_RETURN},
    {"function", 8, TOKEN_FUNCTION},
    {"var", 3, TOKEN_VAR},
    {"switch", 6, TOKEN_SWITCH},
    {"case", 4, TOKEN_CASE},
    {"default", 7, TOKEN_DEFAULT},
    {"break", 5, TOKEN_BREAK},
    {"const", 5, TOKEN_CONST},
    {"void", 4, TOKEN_VOID},
    {"int", 3, TOKEN_INT},
    {"float", 5, TOKEN_FLOAT_TYPE},
    {"double", 6, TOKEN_DOUBLE},
    {"string", 6, TOKEN_STRING_TYPE},
    {"bool", 4, TOKEN_BOOL},
    {"import", 6, TOKEN_IMPORT},
    {"null", 4, TOKEN_NULL},
    {"true", 4, TOKEN_TRUE},
    {"false", 5, TOKEN_FALSE},
    {"enum", 4, TOKEN_ENUM},
    {NULL, 0, TOKEN_ERROR} // 表结束标记
};

// 静态函数前向声明
static int isAtEnd(Lexer *lexer);
static char advance(Lexer *lexer);
static char peek(Lexer *lexer);
static char peekNext(Lexer *lexer);
static int match(Lexer *lexer, char expected);
static void skipWhitespaceAndComments(Lexer *lexer);
static Token makeToken(Lexer *lexer, TokenType type);
static Token errorToken(Lexer *lexer, const char *message);
static int isAlpha(char c);
static int isDigit(char c);
static Token identifier(Lexer *lexer);
static Token number(Lexer *lexer);
static Token string(Lexer *lexer);

// 初始化词法分析器
void initLexer(Lexer *lexer, const char *source)
{
    lexer->source = source;
    lexer->current = source;
    lexer->line = 1;
}

/**
 * @brief 执行词法分析，将源代码转换为令牌序列
 *
 * 此函数对输入的源代码进行词法分析，生成令牌数组。函数会自动管理内存，
 * 当令牌数量超过初始容量时会动态扩展数组大小。
 *
 * @param source 待分析的源代码字符串，必须以null结尾
 * @param tokenCount 输出参数，用于返回生成的令牌数量
 *
 * @return Token* 返回令牌数组的指针，调用者负责释放内存；
 *                如果内存分配失败则返回NULL
 *
 * @note 返回的令牌数组包含源代码的所有令牌，包括最后的EOF令牌
 * @note 如果函数执行失败，*tokenCount将被设置为0
 * @note 调用者需要负责释放返回的令牌数组以及数组中每个令牌的内存
 *
 * @warning 如果内存分配失败，函数会自动清理已分配的资源
 */
Token *performLexicalAnalysis(const char *source, int *tokenCount)
{
    Lexer lexer;
    initLexer(&lexer, source);

    // 分配初始令牌数组
    int capacity = 100;
    Token *tokens = malloc(sizeof(Token) * capacity);
    if (tokens == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        *tokenCount = 0;
        return NULL;
    }

    int count = 0;
    Token token;

    do
    {
        token = nextToken(&lexer);

        // 检查是否需要扩展数组
        if (count >= capacity)
        {
            capacity *= 2;
            Token *newTokens = realloc(tokens, sizeof(Token) * capacity);
            if (newTokens == NULL)
            {
                fprintf(stderr, "内存重新分配失败\n");
                for (int i = 0; i < count; i++)
                {
                    freeToken(&tokens[i]);
                }
                free(tokens);
                *tokenCount = 0;
                return NULL;
            }
            tokens = newTokens;
        }

        tokens[count++] = token;
    } while (token.type != TOKEN_EOF);

    *tokenCount = count;
    return tokens;
}

// 检查是否到达源代码末尾
static int isAtEnd(Lexer *lexer)
{
    return *lexer->current == '\0';
}

// 向前移动一个字符并返回它
static char advance(Lexer *lexer)
{
    return *(lexer->current++);
}

// 查看当前字符但不移动
static char peek(Lexer *lexer)
{
    return *lexer->current;
}

// 查看下一个字符但不移动
static char peekNext(Lexer *lexer)
{
    if (isAtEnd(lexer))
        return '\0';
    return lexer->current[1];
}

// 检查当前字符是否与期望的字符匹配，如果匹配则前进
static int match(Lexer *lexer, char expected)
{
    if (isAtEnd(lexer))
        return 0;
    if (*lexer->current != expected)
        return 0;

    lexer->current++;
    return 1;
}

/**
 * 跳过词法分析器当前位置的空白字符和注释
 *
 * 该函数会连续跳过以下内容：
 * - 空白字符：空格、回车符、制表符
 * - 换行符（同时更新行号计数器）
 * - 单行注释
 * - 多行注释
 *
 * @param lexer 指向词法分析器结构的指针
 *
 * @note 函数会自动处理嵌套在多行注释中的换行符，正确维护行号计数
 * @note 如果遇到单独的 '/' 字符（不是注释开始），函数会停止并返回
 * @note 对于未结束的多行注释，函数会安全地处理到文件末尾
 */
static void skipWhitespaceAndComments(Lexer *lexer)
{
    for (;;)
    {
        char c = peek(lexer);

        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(lexer);
            break;

        case '\n':
            lexer->line++;
            advance(lexer);
            break;

        case '/':
            if (peekNext(lexer) == '/')
            {
                // 单行注释，跳过整行
                while (peek(lexer) != '\n' && !isAtEnd(lexer))
                {
                    advance(lexer);
                }
            }
            else if (peekNext(lexer) == '*')
            {
                // 多行注释
                advance(lexer); // 跳过 '/'
                advance(lexer); // 跳过 '*'

                while (!isAtEnd(lexer) && !(peek(lexer) == '*' && peekNext(lexer) == '/'))
                {
                    if (peek(lexer) == '\n')
                        lexer->line++;
                    advance(lexer);
                }

                if (!isAtEnd(lexer))
                {
                    advance(lexer); // 跳过 '*'
                    advance(lexer); // 跳过 '/'
                }
            }
            else
            {
                return; // 这是除法运算符，不是注释
            }
            break;

        default:
            return;
        }
    }
}

/**
 * 创建一个新的词法单元(Token)
 *
 * 此函数根据给定的词法分析器状态和词法单元类型创建一个新的Token。
 * 它会计算从源码起始位置到当前位置的词素长度，分配内存来存储词素字符串，
 * 并设置相应的行号信息。函数执行完毕后会重置词法分析器的源码指针。
 *
 * @param lexer 指向词法分析器结构体的指针，包含当前解析状态
 * @param type 要创建的词法单元类型
 * @return 返回创建的Token结构体，包含类型、词素字符串和行号信息
 *
 * @note 调用者需要负责释放返回Token中lexeme字段分配的内存
 * @warning 确保lexer->current >= lexer->source，否则可能导致未定义行为
 */
static Token makeToken(Lexer *lexer, TokenType type)
{
    Token token;
    token.type = type;

    // 计算词素长度并复制
    int length = lexer->current - lexer->source;

    // 分配内存并复制词素
    token.lexeme = (char *)malloc(length + 1);
    strncpy(token.lexeme, lexer->source, length);
    token.lexeme[length] = '\0';

    // 保存行号信息
    token.line = lexer->line;

    // 重置源指针为当前位置
    lexer->source = lexer->current;

    return token;
}

/**
 * 创建一个错误类型的Token
 *
 * 此函数用于在词法分析过程中遇到错误时创建一个错误Token。
 * 该Token包含错误类型、错误消息和出错的行号信息。
 *
 * @param lexer 指向词法分析器实例的指针，用于获取当前行号
 * @param message 错误消息字符串，描述具体的错误信息
 * @return Token 返回一个类型为TOKEN_ERROR的Token结构体
 *
 * @note 函数会为错误消息分配内存，调用者需要确保在适当时候释放Token中的lexeme内存
 * @warning 如果内存分配失败，token.lexeme将为NULL
 */
static Token errorToken(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;

    size_t messageLen = strlen(message);
    token.lexeme = (char *)malloc(messageLen + 1);
    if (token.lexeme != NULL)
    {
        strcpy(token.lexeme, message);
    }

    token.line = lexer->line;

    return token;
}

// 判断字符是否为字母或下划线
static int isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

// 判断字符是否为数字
static int isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// 处理标识符和关键字
static Token identifier(Lexer *lexer)
{
    // 记录起始位置
    const char *start = lexer->current - 1; // 退回一个字符，刚刚被advance消费掉的字符

    while (isAlpha(peek(lexer)) || isDigit(peek(lexer)))
    {
        advance(lexer);
    }

    // 计算标识符长度
    int length = lexer->current - start;

    // 检查是否为关键字
    for (int i = 0; keywords[i].keyword != NULL; i++)
    {
        if (length == keywords[i].length &&
            strncmp(start, keywords[i].keyword, length) == 0)
        {
            // 匹配到关键字
            return makeToken(lexer, keywords[i].type);
        }
    }

    // 不是关键字，就是普通标识符
    return makeToken(lexer, TOKEN_IDENTIFIER);
}

// 处理数字（整数和浮点数）
static Token number(Lexer *lexer)
{
    while (isDigit(peek(lexer)))
    {
        advance(lexer);
    }

    // 检查小数点
    if (peek(lexer) == '.' && isDigit(peekNext(lexer)))
    {
        advance(lexer); // 跳过小数点

        while (isDigit(peek(lexer)))
        {
            advance(lexer);
        }

        Token token = makeToken(lexer, TOKEN_FLOAT);
        token.value.floatValue = atof(token.lexeme);
        return token;
    }

    Token token = makeToken(lexer, TOKEN_INTEGER);
    token.value.intValue = atoi(token.lexeme);
    return token;
}

/**
 * 解析字符串字面量
 *
 * 从当前位置开始解析一个由双引号包围的字符串字面量，支持标准的转义序列。
 * 函数会动态分配内存来存储解析后的字符串内容，并处理各种转义字符。
 *
 * 支持的转义序列：
 * - \n: 换行符
 * - \t: 制表符
 * - \r: 回车符
 * - \\: 反斜杠
 * - \": 双引号
 * - \0: 空字符
 * - \b: 退格符
 * - \f: 换页符
 * - \v: 垂直制表符
 * - \a: 响铃符
 * - \/: 斜杠
 * - 其他: 保持原样
 *
 * @param lexer 词法分析器指针，包含当前解析状态
 * @return Token 返回字符串类型的token，如果解析失败则返回错误token
 *
 * @note 函数会为字符串内容分配内存，调用者需要负责释放token.value.stringValue
 * @note 如果遇到未终止的字符串或内存分配失败，会返回相应的错误token
 * @note 初始缓冲区大小为256字节，根据需要自动扩展
 */
static Token string(Lexer *lexer)
{

    // 分配缓冲区
    int bufferSize = 256;
    char *buffer = malloc(bufferSize);
    if (buffer == NULL)
    {
        return errorToken(lexer, "Memory allocation failed.");
    }

    int bufferPos = 0;

    while (peek(lexer) != '"' && !isAtEnd(lexer))
    {
        char c = peek(lexer);

        if (c == '\\')
        {
            // 处理转义字符
            advance(lexer); // 跳过反斜杠

            if (isAtEnd(lexer))
            {
                free(buffer);
                return errorToken(lexer, "Unterminated string escape sequence.");
            }

            char escaped = advance(lexer);
            char actualChar;

            switch (escaped)
            {
            case 'n':
                actualChar = '\n';
                break;
            case 't':
                actualChar = '\t';
                break;
            case 'r':
                actualChar = '\r';
                break;
            case '\\':
                actualChar = '\\';
                break;
            case '"':
                actualChar = '"';
                break;
            case '0':
                actualChar = '\0';
                break;
            case 'b':
                actualChar = '\b'; // 退格符
                break;
            case 'f':
                actualChar = '\f'; // 换页符
                break;
            case 'v':
                actualChar = '\v'; // 垂直制表符
                break;
            case 'a':
                actualChar = '\a'; // 响铃符
                break;
            case '/':
                actualChar = '/'; // 斜杠
                break;
            default:
                // 未知转义序列，保持原样
                actualChar = escaped;
                break;
            }

            // 确保缓冲区足够大
            if (bufferPos >= bufferSize - 1)
            {
                bufferSize *= 2;
                char *newBuffer = realloc(buffer, bufferSize);
                if (newBuffer == NULL)
                {
                    free(buffer);
                    return errorToken(lexer, "Memory allocation failed.");
                }
                buffer = newBuffer;
            }

            buffer[bufferPos++] = actualChar;
        }
        else
        {
            // 普通字符
            if (bufferPos >= bufferSize - 1)
            {
                bufferSize *= 2;
                char *newBuffer = realloc(buffer, bufferSize);
                if (newBuffer == NULL)
                {
                    free(buffer);
                    return errorToken(lexer, "Memory allocation failed.");
                }
                buffer = newBuffer;
            }

            buffer[bufferPos++] = advance(lexer);
        }
    }

    if (isAtEnd(lexer))
    {
        free(buffer);
        return errorToken(lexer, "Unterminated string.");
    }

    // 消耗结束的引号
    advance(lexer);

    // 添加字符串结束符
    buffer[bufferPos] = '\0';

    // 创建 token
    Token token = makeToken(lexer, TOKEN_STRING);

    // 复制处理后的字符串到 value.stringValue
    token.value.stringValue = malloc(strlen(buffer) + 1);
    if (token.value.stringValue != NULL)
    {
        strcpy(token.value.stringValue, buffer);
    }

    // 释放临时缓冲区
    free(buffer);

    return token;
}

/**
 * 词法分析器的核心函数，从输入源中获取下一个词法单元
 *
 * 该函数实现了完整的词法分析逻辑，能够识别和返回各种类型的词法单元：
 * - 标识符和关键字
 * - 数字字面量
 * - 单字符操作符：括号、分隔符、算术运算符等
 * - 双字符操作符：++、--、==、!=、<=、>=、&&、||
 * - 字符串字面量
 * - 文件结束标记
 *
 * 处理流程：
 * 1. 跳过空白字符和注释
 * 2. 记录当前位置作为词法单元的起始位置
 * 3. 检查是否到达文件末尾
 * 4. 根据当前字符的类型进行相应的词法分析
 * 5. 返回构造的词法单元或错误信息
 *
 * @param lexer 词法分析器实例的指针，包含输入源和当前位置信息
 * @return Token 返回识别到的词法单元，包括类型、位置和长度信息
 *
 * @note 对于单独出现的 '&' 和 '|' 字符会返回错误，因为它们必须成对出现
 * @note 遇到无法识别的字符时会返回通用错误信息
 */
Token nextToken(Lexer *lexer)
{
    skipWhitespaceAndComments(lexer);

    lexer->source = lexer->current;

    if (isAtEnd(lexer))
    {
        return makeToken(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    // 标识符
    if (isAlpha(c))
    {
        return identifier(lexer);
    }

    // 数字
    if (isDigit(c))
    {
        return number(lexer);
    }

    switch (c)
    {
    // 单字符标记
    case '(':
        return makeToken(lexer, TOKEN_LPAREN);
    case ')':
        return makeToken(lexer, TOKEN_RPAREN);
    case '{':
        return makeToken(lexer, TOKEN_LBRACE);
    case '}':
        return makeToken(lexer, TOKEN_RBRACE);
    case '[':
        return makeToken(lexer, TOKEN_LBRACKET);
    case ']':
        return makeToken(lexer, TOKEN_RBRACKET);
    case ';':
        return makeToken(lexer, TOKEN_SEMICOLON);
    case ',':
        return makeToken(lexer, TOKEN_COMMA);
    case ':':
        return makeToken(lexer, TOKEN_COLON);
    case '+':
        if (match(lexer, '+'))
        {
            return makeToken(lexer, TOKEN_PLUS_PLUS);
        }
        return makeToken(lexer, TOKEN_PLUS);
    case '-':
        if (match(lexer, '-'))
        {
            return makeToken(lexer, TOKEN_MINUS_MINUS);
        }
        return makeToken(lexer, TOKEN_MINUS);
    case '*':
        return makeToken(lexer, TOKEN_MULTIPLY);
    case '/':
        return makeToken(lexer, TOKEN_DIVIDE);
    case '%':
        return makeToken(lexer, TOKEN_MODULO);

    // 可能是单字符或双字符的标记
    case '=':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_EQ : TOKEN_ASSIGN);
    case '!':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_NE : TOKEN_NOT);
    case '<':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_LE : TOKEN_LT);
    case '>':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_GE : TOKEN_GT);
    case '&':
        if (match(lexer, '&'))
        {
            return makeToken(lexer, TOKEN_AND);
        }
        return errorToken(lexer, "Unexpected character '&'.");
    case '|':
        if (match(lexer, '|'))
        {
            return makeToken(lexer, TOKEN_OR);
        }
        return errorToken(lexer, "Unexpected character '|'.");
    // 字符串
    case '"':
        return string(lexer);
    }

    return errorToken(lexer, "Unexpected character.");
}

/**
 * 释放Token结构体及其相关内存
 *
 * 此函数负责安全地释放Token结构体中动态分配的内存，包括词素(lexeme)
 * 和字符串类型的值。函数会检查指针的有效性，防止重复释放或空指针访问。
 *
 * @param token 指向要释放的Token结构体的指针，可以为NULL
 *
 * @note 函数会将释放后的指针设置为NULL，防止悬空指针
 * @note 对于字符串类型的token，会额外释放其字符串值的内存
 * @note 传入NULL指针是安全的，函数会直接返回
 */
void freeToken(Token *token)
{
    if (token == NULL)
        return;

    if (token->lexeme != NULL)
    {
        free(token->lexeme);
        token->lexeme = NULL;
    }

    if (token->type == TOKEN_STRING && token->value.stringValue != NULL)
    {
        free(token->value.stringValue);
        token->value.stringValue = NULL;
    }
}

// 获取标记类型的字符串表示
const char *getTokenName(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_INTEGER:
        return "INTEGER";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_MODULO:
        return "MODULO";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_EQ:
        return "EQUAL";
    case TOKEN_NE:
        return "NOT_EQUAL";
    case TOKEN_LT:
        return "LESS_THAN";
    case TOKEN_LE:
        return "LESS_EQUAL";
    case TOKEN_GT:
        return "GREATER_THAN";
    case TOKEN_GE:
        return "GREATER_EQUAL";
    case TOKEN_LPAREN:
        return "LEFT_PAREN";
    case TOKEN_RPAREN:
        return "RIGHT_PAREN";
    case TOKEN_LBRACE:
        return "LEFT_BRACE";
    case TOKEN_RBRACE:
        return "RIGHT_BRACE";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_FUNCTION:
        return "FUNCTION";
    case TOKEN_VOID:
        return "VOID";
    case TOKEN_INT:
        return "INT";
    case TOKEN_FLOAT_TYPE:
        return "FLOAT_TYPE";
    case TOKEN_DOUBLE:
        return "DOUBLE";
    case TOKEN_STRING_TYPE:
        return "STRING_TYPE";
    case TOKEN_NOT:
        return "NOT";
    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_BOOL:
        return "BOOL";
    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_CONST:
        return "CONST";
    case TOKEN_SWITCH:
        return "SWITCH";
    case TOKEN_CASE:
        return "CASE";
    case TOKEN_DEFAULT:
        return "DEFAULT";
    case TOKEN_ENUM:
        return "ENUM";
    case TOKEN_DO:
        return "DO";
    case TOKEN_BREAK:
        return "BREAK";
    default:
        return "UNKNOWN";
    }
}