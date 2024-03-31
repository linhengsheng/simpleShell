#include "bin.h"

void sigchld_handler(int signo) {
    (void)signo;  // 防止编译器警告

    int status;
    pid_t pid;

    // 循环等待所有子进程退出
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    }
}

int back_exe(const char* input, int split_index){

    int len_in = strlen(input);
    if(len_in - 1 > split_index){
        perror("无效的命令\n");
        return 1;
    }
    int len_cur = split_index;
    char commandStr[len_cur];
    strncpy(commandStr, input, len_cur - 1);
    commandStr[len_cur - 1] = '\0';
    // printf("%s\n", commandStr);
    
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子进程
        if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
            perror("signal");
            exit(EXIT_FAILURE);
        }
        // 创建新的会话，并脱离终端
        if (setsid() == -1) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }
        // 判断是什么命令
        int index = 0;
        int type = -1;
        seek_first_split_char(commandStr, len_cur, &index, &type);
        if(type != -1){ // 管道命令或者 & 命令等集成多个命令的命令在这里执行
            switch (type)
            {
            case 0: // |
                if(Pipe(commandStr, index, -1)==0)
                    exit(EXIT_SUCCESS);
                break;
            case 1: // &
                perror("无效的后台命令\n");
                exit(EXIT_FAILURE);
                break;
            case 2: // >
            case 3: // <
            case 4: // >>
            case 5: // <<
                if(redirect(commandStr, index, type, -1)==0);
                    exit(EXIT_SUCCESS);
                break;
            default:
                perror("未知的命令\n");
                exit(EXIT_FAILURE);
                break;
            }
        }
        // 运行后台命令
        Command* command = read_command(commandStr, len_cur);
        find_command_binary(command, commands, 0, commands_size-1);
        freeCommand(command);
        exit(EXIT_SUCCESS);
    }

    // 父进程不等待子进程结束

    // 注册 SIGCHLD 信号处理函数
    signal(SIGCHLD, sigchld_handler);
    waitpid(pid, NULL, WNOHANG);
    // printf("& exit\n");
    return 0;
}