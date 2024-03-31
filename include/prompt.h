// prompt.h
#ifndef PROMPT_H
#define PROMPT_H
#include "head.h"
#include "variable.h"
// 打印当前用户名和主机名以及当前工作目录
int init_prompt(int* len);

// 从终端获取相应的命令
char* getDynamicString(int *s, const int len_prompt);
#endif
