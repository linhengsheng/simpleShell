#include "bin.h"
#include <stdio.h>
#include <unistd.h>
int pwd(const Command* command){
    char content[1024] = {0};
    // 获取进程当前目录的函数
    getcwd(content, sizeof(content));
    printf("%s\n", content);
    return 1;
}