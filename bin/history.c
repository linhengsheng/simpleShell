#include "bin.h"

#define MAX_HISTORY_SIZE 100
#define filename "/home/yueyueyue/Code/shell/resources/history.txt"

bool is_max = false;
// 初始化历史记录
void initHistory(History* history) {
    if(history->commands == NULL)
    {
        char** temp = NULL;
        temp = (char**)malloc(MAX_HISTORY_SIZE*sizeof(char*));
        history->commands = temp;
        if(history->commands == NULL){
            history->commands = temp;
            printf("\033[0;31m内存分配失败\033[0m\n");
            return;
        }
    }
    history->count = 0;
    FILE* file = fopen(filename, "a");
    fclose(file);
}

// 添加命令到历史记录
void addToHistory(History* history, const char* command) {
    // 添加命令记录
    if (history->count < MAX_HISTORY_SIZE) {
        history->commands[history->count++] = strdup(command); // 创建command副本，动态分配内存
    } else {
        is_max = true;
        // 如果历史记录已满，删除最早的命令
        free(history->commands[0]);
        // 将命令向前移动
        memmove(history->commands, history->commands + 1, (MAX_HISTORY_SIZE - 1) * sizeof(char*));  // memmove（void*dest, void*src, size_t n）用于将src开始的n字节数据移动到dest
        // 添加新命令到最后
        history->commands[MAX_HISTORY_SIZE - 1] = strdup(command);
    }

    // 保存到文件
    FILE* file = fopen(filename, "a");  // "a" 追加模式
    if (file != NULL) {
        fprintf(file, "%s\n", command);
        fclose(file);
    } else {
        fprintf(stderr, "\033[0;31mError opening file for appending: %s\033[0m\n", filename);
    }
}
void loadHistory(History* history){
    FILE* file = fopen(filename, "rb");
    if(file == NULL){
        fprintf(stderr, "\033[0;31mError opening file for reading: %s\n\033[0m", filename);
    }
    int bufferSize = 100;
    char buffer[bufferSize];
    bool pindex = true;
    while(fgets(buffer, sizeof(buffer), file)){
        int size = strlen(buffer);
        // printf("%d\n", size);
        if(size < bufferSize - 1){ // 未读满的情况
            buffer[size - 1] = '\0';
            if(history->commands[history->count] == NULL){ // 未读满并且是空行
                if (history->count < MAX_HISTORY_SIZE) {
                    history->commands[history->count++] = strdup(buffer); // 创建command副本，动态分配内存

                } else {
                    is_max = true;
                    free(history->commands[0]);
                    memmove(history->commands, history->commands + 1, (MAX_HISTORY_SIZE - 1) * sizeof(char*));
                    history->commands[MAX_HISTORY_SIZE - 1] = strdup(buffer);
                }
            }
            else{ // 未读满并且不是空行
                history->commands[history->count] = realloc(history->commands[history->count], strlen(history->commands[history->count]) + strlen(buffer) + 2);
                if(history->commands[history->count] == NULL){
                    printf("内存分配失败\n");
                }  
                strcat(history->commands[history->count], buffer);
                history->count++;
            }
        }
        else if(size = bufferSize - 1){ // 已读满的情况
            if(buffer[size - 1] == '\n'){ // 已读满但是完整的一行
                buffer[size - 1] = '\0';
                if (history->count < MAX_HISTORY_SIZE) {
                    history->commands[history->count++] = strdup(buffer); // 创建command副本，动态分配内存
                } else {
                    is_max = true;
                    free(history->commands[0]);
                    memmove(history->commands, history->commands + 1, (MAX_HISTORY_SIZE - 1) * sizeof(char*));
                    history->commands[MAX_HISTORY_SIZE - 1] = strdup(buffer);
                }
            }
            else{ // 已读满但是未完整的一行
                if(history->commands[history->count]!=NULL) // 已读满但是已有内容
                {
                    history->commands[history->count] = realloc(history->commands[history->count], strlen(history->commands[history->count]) + strlen(buffer) + 2);
                    if(history->commands[history->count] == NULL){
                        printf("内存分配失败\n");
                    }
                    strcat(history->commands[history->count], buffer);
                }
                else{ // 已读满但是没有内容
                    if (history->count < MAX_HISTORY_SIZE) {
                        history->commands[history->count] = strdup(buffer); // 创建command副本，动态分配内存

                    } else {
                        is_max = true;
                        free(history->commands[0]);
                        memmove(history->commands, history->commands + 1, (MAX_HISTORY_SIZE - 1) * sizeof(char*));
                        history->commands[MAX_HISTORY_SIZE - 1] = strdup(buffer);
                        history->count--;
                    }
                }
            }
        }
    }
    fclose(file);
}
// 打印历史记录
void printHistory(const History* history) {
    for (int i = 0; i < history->count; ++i) {
        printf("[%d]: %s\n", i + 1, history->commands[i]);
    }
}

void printAllHistory(){
    FILE* file = fopen(filename, "rb");
    if(file == NULL){
        fprintf(stderr, "\033[0;31mError opening file for reading: %s\033[0m\n", filename);
    }
    int bufferSize = 100;
    char buffer[bufferSize];
    bool pindex = true;
    int index = 1;
    while(fgets(buffer, sizeof(buffer), file)){
        if(pindex){
            printf("[%d]: ", index++);
        }
        int size = strlen(buffer);
        if(size == bufferSize - 1){
            pindex = false;
        }
        else pindex = true;
        printf("%s", buffer);

    }
    fclose(file);
}
int deleteHistory(History* history, int index){
    if(index <= 0 || index > history->count){
        return 1;
    }
    index--;
    if(is_max){
        index--;
    }
    free(history->commands[index]);
        // 将命令向前移动
    memmove(history->commands + index, history->commands + index + 1, (MAX_HISTORY_SIZE - index - 1) * sizeof(char*));  // memmove（void*dest, void*src, size_t n）用于将src开始的n字节数据移动到dest
    // 添加新命令到最后
    history->count--;
}
// 释放历史记录内存
void freeHistory(History* history) {
    for (int i = 0; i < history->count; ++i) {
        free(history->commands[i]);
    }
    history->count = 0;
    if(history->commands != NULL)
        free(history->commands);
}
void clearHistory(History* history){
    for (int i = 0; i < history->count; ++i) {
        free(history->commands[i]);
    }
    history->count = 0;
}
// -d -a -c
int history(const Command* command){
    switch(command->num_params){ // 根据参数数量进行处理
        case 0:
            if(command->params->num_values==1){ // 只支持没有参数或者一个参数
                if(strcmp(command->params->key, "-d") == 0){
                    if(command->params->value == NULL) break;
                    if(strcmp(command->params->value[0], "all") == 0){
                        clearHistory(&historyCommands);
                        is_max = false;
                        break;
                    }
                    char *endptr;
                    int num = (int)strtol(command->params->value[0], &endptr, 10); // 将字符串数字转换成int型整数
                    if(*endptr != '\0') break;
                    deleteHistory(&historyCommands, num);
                    break;
                }
                else if(strcmp(command->params->key, "-a") == 0){
                    printAllHistory();
                    break;
                }
                else if(strcmp(command->params->key, "-c") == 0){
                    remove(filename);
                    clearHistory(&historyCommands);
                    is_max = false;
                    break;
                }
            }break;
        default: printHistory(&historyCommands);break;
    }
    return 0;
}
