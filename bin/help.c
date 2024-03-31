#include "bin.h"
static const char* help_doc_prefix = "/home/yueyueyue/Code/shell/resources";
#define FILE_NAME_LEN 256
const char commandPrompt[] = "切换工作目录命令 cd\n后台运行命令 & help此命令时, 请使用: help back\n管道命令 | help此命令时, 请使用: help pipe\n历史记录命令 history\n显示指定目录文件命令 ls\n移动文件/文件夹命令 mv\n复制文件/文件夹命令 cp\n重定向命令 >/</>> help此命令时, 请使用: help redirect\n显示当前完整工作目录命令 pwd\n显示用户目录的树形结构命令 tree\n显示当前用户工作进程命令 ps\n删除文件/文件夹命令 rm\n执行二进制文件命令 <yourfile>\n如需查看详细命令信息, 请使用help <command>\n";
int help(const Command* command){
    if(command->num_params == -1){
        printf("%s\n",commandPrompt);
        return 0;
    }
    if(command->num_params > 0){
        perror("\033[0;31m参数过多\033[0m\n");
        return 1;
    }
    FILE* fp;
    char c;
    char file_name[FILE_NAME_LEN] = {0};
    snprintf(file_name, sizeof(file_name), "%s/help_%s.txt", help_doc_prefix, command->params->value[0]);

    if (access(file_name, F_OK) != 0) {
        perror("\033[0;31m不是有效的指令\033[0m\n");
        return 1;
    }

    fp = fopen(file_name, "r");  // 读取文件
    if (fp == NULL) {
        perror("\033[0;31mError: help fopen\033[0m\n");
        return 1;
    }

    while ((c = fgetc(fp)) != EOF) {  // 读取文件内容，直到文件末尾
      putchar(c);                     // 输出字符
    }
    printf("\n");
    fclose(fp);  // 关闭文件

    return 0;    
}