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
    {"while", 5, TOKEN_WHILE},
    {"for", 3, TOKEN_FOR},
    {"return", 6, TOKEN_RETURN},
    {"function", 8, TOKEN_FUNCTION},
    {"var", 3, TOKEN_VAR},
    {"void", 4, TOKEN_VOID},
    {"int", 3, TOKEN_INT},
    {"float", 5, TOKEN_FLOAT_TYPE},
    {"string", 6, TOKEN_STRING_TYPE},
    {"bool", 4, TOKEN_BOOL},
    {"import", 6, TOKEN_IMPORT},
    {"null", 4, TOKEN_NULL},
    {"true", 4, TOKEN_TRUE},
    {"false", 5, TOKEN_FALSE},
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

// 执行词法分析
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

// 跳过空白字符和注释
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

// 创建一个新的标记
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

    // 去除词素开头的空白字符
    char *trimmed = token.lexeme;
    while (isspace(*trimmed))
        trimmed++;

    if (trimmed != token.lexeme)
    {
        // 如果开头有空白，移动字符串
        memmove(token.lexeme, trimmed, strlen(trimmed) + 1);
    }

    // 重置源指针为当前位置 (只做一次)
    lexer->source = lexer->current;

    return token;
}

// 创建错误标记
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

// 处理字符串
static Token string(Lexer *lexer)
{
    // 跳过开始的引号
    advance(lexer);

    const char *start = lexer->current;

    while (peek(lexer) != '"' && !isAtEnd(lexer))
    {
        if (peek(lexer) == '\n')
            lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer))
    {
        return errorToken(lexer, "Unterminated string.");
    }

    // 计算字符串内容长度
    int length = lexer->current - start;

    // 跳过结束的引号
    advance(lexer);

    Token token = makeToken(lexer, TOKEN_STRING);

    // 为字符串内容分配内存
    token.value.stringValue = (char *)malloc(length + 1);
    strncpy(token.value.stringValue, start, length);
    token.value.stringValue[length] = '\0';

    return token;
}

// 获取下一个标记
Token nextToken(Lexer *lexer)
{
    skipWhitespaceAndComments(lexer);

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

// 释放标记占用的内存
void freeToken(Token *token)
{
    if (token->lexeme)
    {
        free(token->lexeme);
    }

    if (token->type == TOKEN_STRING && token->value.stringValue)
    {
        free(token->value.stringValue);
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
    case TOKEN_DO:
        return "DO";
    case TOKEN_BREAK:
        return "BREAK";
    default:
        return "UNKNOWN";
    }
}