#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_utils.h"

// 读取文件内容
char *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "无法打开文件 \"%s\".\n", path);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // 分配内存
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "无法分配足够内存读取文件 \"%s\".\n", path);
        fclose(file);
        return NULL;
    }

    // 读取文件内容
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "无法读取文件 \"%s\".\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 添加字符串终止符
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}