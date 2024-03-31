// bin.h 包括所有功能的头文件
#ifndef BIN_H
#define BIN_H
#include "head.h"
#include "variable.h"
#include "eval.h"
// command &: 后台运行程序，不阻塞当前的shell, 同时终止信号将不能终止后台程序的运行，支持所有二进制文件或已实现命令
int back_exe(const char* input, int split_index);
// 执行二进制文件, 此处不使用execvp，因此不会使用bash的解释器
int exe(const Command* command);
// 切换工作目录，支持相对路径
int cd(const Command* command);
// 解析相对路径
int findRelative(const char* str, char* result);
void getDir(int lastindex, char* str, int len);
// 获取根路径
void getRoot(char*temp, int len);
void ReadRoot(const char*str, char* root, int len);
// 复制文件或目录文件（递归实现）
int copyFile(const char *src, const char *dst);
// -r表示复制一个文件夹到指定文件夹，无参数表示复制一个文件到另一个文件
// 对于无后缀文件，默认复制到一个文件夹，即使同名，如果希望复制到文件或(并)重命名，请使用-f
// cp是不带文件夹复制，即源文件夹下的所有内容复制到目标文件夹下，而不在目标文件夹下创建与源文件夹同名的目录
// cp <option> <src> <dst>
int cp(const Command* command);

// 命令信息
int help(const Command* command);

// -c: 删除所有历史命令记录 -a：输出所有历史命令的记录 -d [num]: 删除内存的历史命令中的某个命令 -d all：删除内存历史命令的全部，下次运行时会重新加载
int history(const Command* command);

void initHistory(History* history);
void addToHistory(History* history, const char* command);
void freeHistory(History* history);
void loadHistory(History* history);

// 列出指定目录下的文件和子文件夹，蓝色表示文件夹, 白色表示文件
int ls(const Command* command);

// 移动文件或文件夹
int CreateDirectory(const char* path);
// 支持相对路径和完整路径的递归文件夹创建
int createDirectories(const char *path);
// 获取文件上级目录, 没有上级目录: -1 有: 0
int getLastDir(const char* path, char* result, int len);
// 判断文件类型 判断失败: -1 文件: 0 文件夹: 1
int is_Dir_File(const char* path);
// 获取文件扩展名称, 无后缀: -1 错误: 1 有后缀: 0
int getExtendedName(const char* url, char* temp, int len);
// 把src的文件名称拼接到dst目录下
int getFileName(const char*src, char* dst, int len);
// 无参数表示移动一个文件，-r表示移动文件夹
// 如果源文件是无后缀文件，默认目的路径是文件夹路径，即使与源文件同名，也将创建一个同名文件夹，将文件移动到这个同名文件夹下，防止干扰
// mv是不带文件夹移动，即源文件夹下的所有内容移动到目标文件夹下，而不在目标文件夹下创建与源文件夹同名的目录
// 对于无后缀文件，默认移动到一个文件夹，即使同名，如果希望移动到文件或(并)重命名，请使用-f
// mv <-r/''> <src> <dst>
int mv(const Command* command);

// 管道，支持多个（一连串的管道操作）
int Pipe(const char* input, int split_index, int PipeReadFd);

// 重定向 支持重定向输入输出文件，支持与管道一起使用
// command > file: 将command的标准输出重定向到file(覆盖)
// command < file: 将command的标准输入重定向到file
// command >> file: 将command的标准输出重定向到file(追加)
int redirect(const char* input, int split_index, int type, int PipeReadFd);
// 列出进程信息
int ps(const Command* command);
// 显示当前工作目录
int pwd(const Command* command);
// 删除文件或文件夹
// option: -r 或无, -r时递归删除文件夹, 无option时删除文件
// 只支持一次删除一个文件或文件夹
int rm(const Command* command);

// 列出指定目录下的结构
// 支持相对路径
int tree(const Command* command);

#endif