#include "eval.h"
#ifndef CHUNK_SIZE
#define CHUNK_SIZE 10
#endif
#ifndef PARAM_SIZE
#define PARAM_SIZE 1
#endif
char* split_str(const char*a, int len, int start_index, int* end_index, bool *option){
    if(start_index>=len){
        return NULL;
    }
    *end_index = 0;
    int capacity = CHUNK_SIZE;
    int size = 0;
    char* buffer = (char*)malloc(capacity * sizeof(char));  // 分配初始内存
    if (buffer == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    int space_num = 0;
    while(start_index < len && a[start_index] == ' '){
        space_num++;
        start_index++;
    }
    if(a[start_index]=='-')
    {
        if(*option){
            free(buffer);
            return NULL;
        }
        *option = true;
    }
    else{
        *option = false;
    }
    // printf("start_index:%d   len:%d\n", start_index, len);
    while(start_index < len && a[start_index] != ' '){
        buffer[size++] = a[start_index++];
        // 如果达到当前分配的内存容量，则扩展内存
        if (size >= capacity) {
            capacity += CHUNK_SIZE;
            char* temp = (char*)realloc(buffer, capacity * sizeof(char));

            if (temp == NULL) {
                printf("内存分配失败\n");
                free(buffer);
                return NULL;
            }

            buffer = temp;
        }
    }
    // printf("size: %d\n", size);
    if(size==0){
        return NULL;
    }
    *end_index += size + space_num;
    buffer[size] = 0;
    return buffer;

}
Command* read_command(char *input, int len){
    int len_in = strlen(input);
    if(len_in < len){
        len = len_in;
    }
    if(len <= 0){
        return NULL;
    }
    Command* command = (Command*)malloc(sizeof(Command));
    if(command == NULL){
        printf("\033[1;31m内存分配失败\033[0m\n");
        return NULL;
    }

    int end_index;
    bool option = false; //选择是否是命令参数
    char *ret = split_str(input, len, 0, &end_index, &option);
    if(ret==NULL){
        return NULL;
    }
    command->command_name = ret;
    command->num_params = -1;
    int param_capacity = PARAM_SIZE;

    command->params = (CommandParam*)malloc(param_capacity*sizeof(CommandParam));
    command->params[0].key = NULL;
    command->params[0].num_values = 0;
    if (command->params == NULL) {
        printf("\033[1;31m内存分配失败\033[0m\n");
        free(command);
        return NULL;
    }
    int index = end_index + 1;
    int value_capacity = PARAM_SIZE;
    while(true){
        ret = split_str(input, len, index, &end_index, &option);
        if(ret!=NULL){
            if((command->num_params)>=param_capacity-1){
                param_capacity += PARAM_SIZE;
                CommandParam* param_temp = (CommandParam*)realloc(command->params, param_capacity*sizeof(CommandParam));
                if(param_temp==NULL){
                    printf("重新分配内存失败\n");
                    for(int i=0; i <= command->num_params; ++i){
                        if(command->params[i].key!=NULL)
                            free(command->params[i].key);
                        if(command->params[i].value != NULL)
                        {
                            for(int j=0; j < command->params[command->num_params].num_values; j++)
                                free(command->params[i].value[j]);
                            free(command->params[i].value);
                        }
                    }
                    free(command->params);
                    free(command);
                    return NULL;
                }
                command->params = param_temp;
            }
            if(option){
                command->num_params++;
                command->params[command->num_params].key = ret;

                value_capacity = PARAM_SIZE;
                command->params[command->num_params].num_values = 0;
                char** value_temp = (char**)malloc(value_capacity*sizeof(char*));
                if(value_temp==NULL){
                    printf("内存分配失败\n");
                    for(int i=0; i <= command->num_params; ++i){
                        if(command->params[i].key!=NULL)
                            free(command->params[i].key);
                    }
                    free(command->params);
                    free(command);
                    return NULL;
                }
                command->params[command->num_params].value = value_temp;
            }
            else if(!option){
                if(command->params[command->num_params + 1].key == NULL && command->num_params == -1)
                {
                    command->num_params++;
                    value_capacity = PARAM_SIZE;
                    command->params[command->num_params].num_values = 0;
                    char** value_temp = (char**)malloc(value_capacity*sizeof(char*));
                    if(value_temp==NULL){
                        printf("内存分配失败\n");
                        for(int i=0; i <= command->num_params; ++i){
                            if(command->params[i].key!=NULL)
                                free(command->params[i].key);
                        }
                        free(command->params);
                        free(command);
                        return NULL;
                    }
                    command->params[command->num_params].value = value_temp;
                }
                
                // printf("%d    %d   %s\n", command->num_params, command->params[command->num_params].num_values, ret);
                command->params[command->num_params].value[(command->params[command->num_params].num_values)++] = ret;
                if(command->params[command->num_params].num_values >= value_capacity){
                    value_capacity += PARAM_SIZE;
                    // printf("%d\n", value_capacity);
                    char** value_temp = (char**)realloc(command->params[command->num_params].value, value_capacity*sizeof(char*));
                    if(value_temp==NULL){
                        printf("重新分配内存失败\n");
                        for(int i=0; i <= command->num_params; ++i){
                            if(command->params[i].key!=NULL)
                                free(command->params[i].key);
                            if(command->params[i].value != NULL)
                            {
                                for(int j=0; j < command->params[command->num_params].num_values; j++)
                                    free(command->params[i].value[j]);
                                free(command->params[i].value);
                            }
                        }
                        free(command->params);
                        free(command);
                        return NULL;
                    }
                    command->params[command->num_params].value = value_temp;
                }
            }
            index += end_index + 1;
        }
        else{
            if(option){
                option = false;
                command->params[command->num_params].value[command->params[command->num_params].num_values++] = NULL;
                continue;
            }
            break;
        }
    }

    // // 以下打印command结构体， 方便观察
    // printf("Command: %s\n", command->command_name);
    // if(command->num_params==-1){
    //     command->params = NULL;
    // }
    // // printf("%d\n", command->num_params);
    // for(int i=0; i<=command->num_params; i++){
    //     printf("option: %s 参数个数：%d 参数：", command->params[i].key, command->params[i].num_values);
    //     for(int j=0; j < command->params[i].num_values; j++)
    //     {
    //         printf("%s ", command->params[i].value[j]);
    //     }
    //     printf("\n");        
    // }
    return command;
}

int getSplit_str(const char* input, int split_index, char*result, int len){
    int index = -1;
    for(int i = split_index - 1; i >= 0; i--){
        if(input[i] == ' ')
        {
            continue;
        }
        else{
            index = i;
            break;
        }
    }
    if(index == -1 || index + 1 > len){
        return 1;
    }
    for(int i = 0; i <= index; i++){
        result[i] = input[i];
    }
    result[index + 1] = '\0';
    return 0;
}

void seek_first_split_char(char* input, int len, int* first, int* type){
    while(len > 0){
        if(input[len - 1] == ' ' || input[len - 1] == '\n'){
            input[len - 1] = '\0';
            len--;
        }
        else if(input[len - 1] == '&'){
            if(len > 1){
                if(input[len - 2] == ' '){
                    *first = len - 1;
                    *type = 1;
                    return;
                }
            }
            printf("无效的命令\n");
            *type = -1;
            *first = -1;
            return;
        }
        else{
            break;
        }
    }
    char *str = NULL;
    int index = 0;
    int end_index = 0;
    bool option = false;
    while(index<len){
        option = false;
        if(str != NULL){
            free(str);
        }
        str = split_str(input, len, index, &end_index, &option); 
        // printf("!!!\n");
        if(str == NULL){
            // printf("%d   %d\n", index, end_index);
            index += end_index + 1;
            continue;
        }
        int len_str = strlen(str);
        if(strcmp(str, "|")==0){
            if(str != NULL){
                free(str);
            }
            *type = 0;
            *first = index + end_index - len_str;
            return;
        }
        else if(strcmp(str, "&")==0){
            if(str != NULL){
                free(str);
            }
            *type = 1;
            *first = index + end_index - len_str;
            return;
        }
        else if(strcmp(str, ">")==0){
            if(str != NULL){
                free(str);
            }
            *type = 2;
            *first = index + end_index - len_str;
            return;
        }
        else if(strcmp(str, "<")==0){
            if(str != NULL){
                free(str);
            }
            *type = 3;
            *first = index + end_index - len_str;
            return;
        }
        else if(strcmp(str, ">>")==0){
            if(str != NULL){
                free(str);
            }
            *type = 4;
            *first = index + end_index - len_str;
            return;
        }
        else if(strcmp(str, "<<")==0){
            if(str != NULL){
                free(str);
            }
            *type = 5;
            *first = index + end_index - len_str;
            return;
        }
        index += end_index + 1;
    }
    if(str != NULL){
        free(str);
    }
    *type = -1;
    *first = -1;
    return;
}
void find_command_binary(const Command* command, Exe_command *commands, int low, int high) {
    if(command == NULL){
        return;
    }
    while (low <= high) {
        int mid = (low + high) / 2;
        int compare_result = strcmp(command->command_name, commands[mid].command_name);

        if (compare_result == 0) {
            // 找到命令，执行相应的函数
            commands[mid].function(command);
            return;
        } else if (compare_result < 0) {
            // 在左半部分继续查找
            high = mid - 1;
        } else {
            // 在右半部分继续查找
            low = mid + 1;
        }
    }
    if(exe(command) != 0){
        printf("Unknown command: %s\n", command->command_name);
    }
}
int cmd_compare(const void* a, const void* b) {
    const Exe_command* command_a = (const Exe_command*)a;
    const Exe_command* command_b = (const Exe_command*)b;

    return strcmp(command_a->command_name, command_b->command_name);
}
void freeCommand(Command* command){
    if(command!=NULL){
        for(int i=0; i <= command->num_params; ++i){
            if(command->params[i].key!=NULL)
                free(command->params[i].key);
            if(command->params[i].value != NULL)
            {
                for(int j=0; j < command->params[command->num_params].num_values; j++)
                    free(command->params[i].value[j]);
                free(command->params[i].value);
            }
        }
        free(command->params);
        free(command);
    }
}