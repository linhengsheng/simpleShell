#include "main.h"
#include <signal.h>
#define True 1
int main(){
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    qsort(commands, commands_size, command_size, cmd_compare);
    // 可以输出普通命令集合，
    printf("可执行命令如下\n"); // executable commands are as follows
    for(int i=0; i<commands_size; i++){
        printf("%s\n", commands[i].command_name);
    }

    initHistory(&historyCommands);
    loadHistory(&historyCommands);
    while(True){
        int len_prompt = 0;
         //解析并输出当前执行目录(print current workspace directory)
        if(init_prompt(&len_prompt)==1){
            printf("wrong!\n");
            break;
        }; 
        
        int len_input=0;
        char *input = getDynamicString(&len_input, len_prompt);
        if(input==NULL && len_input==-1)break;
        else if(input==NULL && len_input==0)continue;
        // 添加到历史记录 (add command to history)
        addToHistory(&historyCommands, input);
        int index = 0;
        int type = -1;
        seek_first_split_char(input, len_input, &index, &type);
        // 可能去除空格 (remove spaces)
        len_input = strlen(input);
        if(type != -1){ // 管道命令或者 & 命令等集成多个命令的命令在这里执行 (pipe command and other combined command excuted here)
            switch (type)
            {
            case 0: // |
                Pipe(input, index, -1);
                break;
            case 1: // &
                back_exe(input, index);
                break;
            case 2: // >
            case 3: // <
            case 4: // >>
            case 5: // <<
                redirect(input, index, type, -1);
                break;
            default:
                break;
            }
        }
        // 普通命令处理(common commands)
        else{
            Command* command = read_command(input, len_input);
            find_command_binary(command, commands, 0, commands_size-1);
            freeCommand(command);
        }
        if(input!=NULL){
            free(input);
        }
    }
    freeHistory(&historyCommands);
    return 0;
}