// variable.h
#ifndef VARIABLE_H
#define VARIABLE_H
#include "head.h"
// 通用参数结构体
typedef struct {
    char* key;
    int num_values;
    char** value;
} CommandParam;

// 通用命令结构体
typedef struct {
    const char* command_name;
    int num_params;
    CommandParam* params;
} Command;  // 无输入参数时，num_params = -1

// 固有命令结构体, function的返回值用于错误检测，0为正常执行，1或其它状态可以定义错误类型
typedef struct {
    const char* command_name;
    int (*function)(const Command *);
}Exe_command;
// 历史命令
typedef struct {
    char** commands;
    int count;
} History;

extern Exe_command commands[];
extern size_t commands_size;
extern size_t command_size;
extern History historyCommands;
#endif
