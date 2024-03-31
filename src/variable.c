#include "variable.h"
#include "bin.h"
// pipe和&的命令由于要解析多条命令，因此不在普通命令之中执行
Exe_command commands[] = {
    {"ls", ls},
    {"cd", cd},
    {"cp", cp},
    {"help", help},
    {"ls", ls},
    {"mv", mv},
    {"ps", ps},
    {"pwd", pwd},
    {"rm", rm},
    {"tree", tree},
    {"history", history}
};

size_t commands_size = sizeof(commands)/sizeof(commands[0]);
size_t command_size = sizeof(commands[0]);
History historyCommands = {NULL, 0};