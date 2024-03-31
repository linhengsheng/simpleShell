#include "bin.h"

int Pipe(const char* input, int split_index, int PipeReadFd){
    int len_in = strlen(input);
    int len_next = len_in - split_index - 1;
    if(len_next <= 0){
        perror("无效的管道命令\n");
        return 1;
    }
    int len_cur = split_index;
    if(len_cur <= 0){
        perror("无效的管道命令\n");
        return 1;
    }
    char commandStr[len_cur];
    char nextCommandStr[len_next];
    strncpy(commandStr, input, len_cur - 1);
    strncpy(nextCommandStr, input + split_index + 2, len_next - 1);
    commandStr[len_cur - 1] = '\0';
    nextCommandStr[len_next - 1] = '\0';
    int type = -1;
    seek_first_split_char(nextCommandStr, len_next, &split_index, &type);
    if(PipeReadFd == -1){
        // 创建管道
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            return 1;
        }
        // 创建进程
        pid_t child_pid1 = fork();

        if (child_pid1 == -1) {
            perror("fork");
            return 1;
        }
        // pid_1的逻辑
        if(child_pid1 == 0){
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[0]); // 关闭读取端
            // 将写入端与标准输出相连
            if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            Command* command = read_command(commandStr, len_cur);
            find_command_binary(command, commands, 0, commands_size-1);
            freeCommand(command);

            close(STDOUT_FILENO);
            close(pipe_fd[1]);
            exit(EXIT_SUCCESS);
        }   
        // 创建进程
        pid_t child_pid2 = fork();

        if (child_pid2 == -1) {
            perror("fork");
            return 1;
        }
        // pid_2的逻辑
        if(child_pid2 == 0){
            if(type == -1){
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]); // 关闭写入端
                dup2(pipe_fd[0], STDIN_FILENO);
                Command* command = read_command(nextCommandStr, len_next);
                find_command_binary(command, commands, 0, commands_size-1);
                close(pipe_fd[0]);
                exit(EXIT_SUCCESS);
            }
            else if(type == 0){
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]);
                // 把读取端传递给Pipe
                if(Pipe(nextCommandStr, split_index, pipe_fd[0]) == 0)
                    exit(EXIT_SUCCESS);
                else exit(EXIT_FAILURE);
            }
            else{ // 其它类型的命令：重定向
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]);
                if (redirect(nextCommandStr, split_index, type, pipe_fd[0]) == 0)
                    exit(EXIT_SUCCESS);
                else exit(EXIT_FAILURE);
            }
        }
        waitpid(child_pid1, NULL, 0);
        close(pipe_fd[1]);
        waitpid(child_pid2, NULL, 0);
        close(pipe_fd[0]);
        // printf("all exit\n");
        return 0;
    }
    else{
        // 创建管道
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            return 1;
        }
        // 创建进程
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("fork");
            return 1;
        }

        // pid_2的逻辑
        if(child_pid == 0){
            if(type == -1){
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]); // 关闭写入端
                dup2(pipe_fd[0], STDIN_FILENO);
                Command* command = read_command(nextCommandStr, len_next);
                find_command_binary(command, commands, 0, commands_size-1);
                exit(EXIT_SUCCESS);
            }
            else if(type == 0){
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]); // 关闭写入端
                // 把读取端传递给Pipe
                if(Pipe(nextCommandStr, split_index, pipe_fd[0]) == 0)
                    exit(EXIT_SUCCESS);
                else exit(EXIT_FAILURE);
            }
            else{ // 其它类型的命令：重定向
                if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                    perror("signal");
                    exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]);
                if (redirect(nextCommandStr, split_index, type, pipe_fd[0]) == 0)
                    exit(EXIT_SUCCESS);
                else exit(EXIT_FAILURE);
            }
        }
        else{
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                return 1;
            }
            close(pipe_fd[0]); // 关闭读取端
            dup2(PipeReadFd, STDIN_FILENO); // 将读取端与上一个pipe绑定
            // 将写入端与标准输出相连
            if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                return 1;
            }
            Command* command = read_command(commandStr, len_cur);
            find_command_binary(command, commands, 0, commands_size-1);
            freeCommand(command);

            close(STDOUT_FILENO);
            close(pipe_fd[1]);
            waitpid(child_pid, NULL, 0);
            return 0;
        }
    }
    // printf("type is other\n");
    return 0;
}
