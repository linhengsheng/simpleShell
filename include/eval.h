// eval.h
#ifndef EVAL_H
#define EVAL_H
#include "head.h"
#include "variable.h"

int exe(const Command* command);

// 解析普通的单个命令，管道、重定向等特殊命令的解析由相关函数实现
Command* read_command(char *input, int len);
// 读取空格分离的字符串，输入为一个字符串，以及字符串的最大长度
// split_str将从start_index开始索引(包括start_index), 直到整个字符串的末尾或者第一个分离的字符串末尾停止
// 返回一个动态分配的数组地址，包含分割的字符串
// end_index将返回start_index到分割后的字符串的最后一个非空格字符在原字符串的所在位置的距离
// option用于读取命令参数，当option=true时，说明start_index之前读取的分割字符串是option: 如 -r/-d等
// option=false时，将不对字符串做相应的判断，此时split_str可以是一个简单的分割字符串的函数，
// option返回的是: 当前所分割的字符串是否是command的参数, 即是类似 -r的option还是参数的值
char* split_str(const char*a, int len, int start_index, int *end_index, bool *option);

// 获取分隔符之前的字符串
int getSplit_str(const char* input, int split_index, char*result, int len);
// 解析特殊命令（如管道，重定向和后台命令）, 返回的是第一个特殊命令所在的位置以及特殊命令的类型
void seek_first_split_char(char* input, int len, int* first, int* type);

// 二分查找普通命令或二进制文件执行命令并执行
void find_command_binary(const Command* command, Exe_command *commands, int low, int high);

// 按字母排列顺序比较命令名称的cmp，用于qsort，将commands排序之后才能进行二分查找
int cmd_compare(const void* a, const void* b);

// command是动态分配的结构体，需要释放
void freeCommand(Command* command);
#endif
