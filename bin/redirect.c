#include "bin.h"

int w_redirect(const char* input, int split_index, int type, int PipeReadFd);
int in_redirect(const char* input, int split_index, int type, int PipeReadFd);
int many_in_redirect(const char* input, int split_index, int type, int PipeReadFd);
int redirect(const char* input, int split_index, int type, int PipeReadFd){
    switch(type){
        case 2: // >
            w_redirect(input, split_index, type, PipeReadFd);
            break;
        case 3: // <
            in_redirect(input, split_index, type, PipeReadFd);
            break;
        case 4: // >>
            w_redirect(input, split_index, type, PipeReadFd);
            break;
        case 5: // <<
            // many_in_redirect();
            perror("未能解析的命令\n");
            return 1;
            break;
        default:break;
    }
    return 0;
}
int w_redirect(const char* input, int split_index, int type, int PipeReadFd){
    int len_in = strlen(input);
    int len_next = len_in - split_index - 1;
    if(len_next <= 0 || split_index <= 0){
        perror("无效的重定向命令\n");
        return 1;
    }
    // 获取第一个命令
    int len_cur = split_index;
    char commandStr[len_cur];
    if(getSplit_str(input, split_index, commandStr, sizeof(commandStr)) == 1){
        return 1;
    }
    len_cur = strlen(commandStr);

    // 获取分隔符后面的字符串
    char nextCommandStr[len_next];
    strncpy(nextCommandStr, input + split_index + 2, len_next - 1);
    nextCommandStr[len_next - 1] = '\0';
    int trunc = -1;
    if(type == 2){
        trunc = 1;
    }
    else if(type == 4){
        trunc = 0;
    }
    else{
        return 1;
    }
    // printf("command: %s\n", commandStr);
    seek_first_split_char(nextCommandStr, len_next, &split_index, &type);
    if(type == -1){ // 后续无特殊命令
        int end_index = 0;
        bool option = false;
        char* filename;
        char* temp = split_str(nextCommandStr, len_next, 0, &end_index, &option);
        if(temp == NULL){
            printf("\033[31m读取文件名称失败\033[m\n");
            return 1;
        }
        // 解析文件名
        if(temp[0] == '~'){
            char root[255];
            ReadRoot(temp, root, sizeof(root));
            filename = strdup(root);
        }
        else{
            filename = strdup(temp);
        }
        
        option = false;
        if(temp != NULL)
            free(temp);
        temp = split_str(nextCommandStr, len_next, end_index + 1, &end_index, &option);
        if(temp != NULL){
            printf("无效的命令\n");
            free(temp);
            free(filename);
            return 1;
        }

        // 创建文件夹 
        // filename 为相对路径或完整路径
        char lastDir[255];
        if(getLastDir(filename, lastDir, sizeof(lastDir)) == 0){
            DIR* dir = opendir(lastDir);
            if(dir == NULL){ // 目录不存在
                createDirectories(lastDir);
            }
        }
        // 创建文件
        int fd = -1;
        if(trunc == 1){
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); // 覆盖模式
        }
        else if(trunc == 0){
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666); // 追加模式
        }
        if (fd == -1) {
            perror("open");
            free(filename);
            return 1;
        }
        // 保存原始的标准输出文件描述符
        int original_stdout = dup(STDOUT_FILENO);
        int original_stdin = -1;
        // 将标准输出重定向到文件
        dup2(fd, STDOUT_FILENO);
        if(PipeReadFd != -1){ // 前面有管道命令
            original_stdin = dup(STDIN_FILENO);
            dup2(PipeReadFd, STDIN_FILENO); // 将读取端与上一个pipe绑定
        }
        // 执行
        Command* command = read_command(commandStr, len_cur);
        find_command_binary(command, commands, 0, commands_size-1);
        // 关闭文件描述符
        close(fd);
        
        // 恢复标准输出到原始状态
        dup2(original_stdout, STDOUT_FILENO);
        if(PipeReadFd != -1){
            dup2(original_stdin, STDIN_FILENO);
        }
        free(filename);
        return 0;
    }
    printf("无法解析的命令: %s\n", nextCommandStr);
    return 1;
}

int in_redirect(const char* input, int split_index, int type, int PipeReadFd){
    if(PipeReadFd != -1){
        perror("无效的重定向命令\n");
        return 1;
    }
    int len_in = strlen(input);
    int len_next = len_in - split_index - 1;
    if(len_next <= 0 || split_index <= 0){
        perror("无效的重定向命令\n");
        return 1;
    }
    // 获取第一个命令
    int len_cur = split_index;
    char commandStr[len_cur];
    if(getSplit_str(input, split_index, commandStr, sizeof(commandStr)) == 1){
        return 1;
    }
    len_cur = strlen(commandStr);

    // 获取分隔符后面的字符串
    char nextCommandStr[len_next];
    strncpy(nextCommandStr, input + split_index + 2, len_next - 1);
    nextCommandStr[len_next - 1] = '\0';
    seek_first_split_char(nextCommandStr, len_next, &split_index, &type);
    if(type == -1){ // 后续没有特殊命令
        int end_index = 0;
        bool option = false;
        char* filename;
        char* temp = split_str(nextCommandStr, len_next, 0, &end_index, &option);
        if(temp == NULL){
            printf("\033[31m读取文件名称失败\033[m\n");
            return 1;
        }
        // 解析文件名
        if(temp[0] == '~'){
            char root[255];
            ReadRoot(temp, root, sizeof(root));
            filename = strdup(root);
        }
        else{
            filename = strdup(temp);
        }
        
        option = false;
        if(temp != NULL)
            free(temp);
        temp = split_str(nextCommandStr, len_next, end_index + 1, &end_index, &option);
        if(temp != NULL){
            printf("无效的命令\n");
            free(temp);
            free(filename);
            return 1;
        }
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            free(filename);
            return 1;
        }
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("fork");
            free(filename);
            return 1;
        }
        // pid_1的逻辑
        if(child_pid == 0){
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[1]); // 关闭写入端
            int original_stdin = dup(STDIN_FILENO);
            dup2(pipe_fd[0], STDIN_FILENO);
            Command* command = read_command(commandStr, len_cur);
            find_command_binary(command, commands, 0, commands_size-1);
            close(pipe_fd[0]);
            dup2(original_stdin, STDIN_FILENO);
            exit(EXIT_SUCCESS);
        }
        else{
            close(pipe_fd[0]);
            // 向管道写入文件内容
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                perror("fopen");
                close(pipe_fd[1]);
                free(filename);
                return 1;
            }

            char buffer[255];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(pipe_fd[1], buffer, bytesRead);
            }

            // 关闭写入端
            close(pipe_fd[1]);

            // 等待子进程结束
            waitpid(child_pid, NULL, 0);
            // 关闭文件
            fclose(file);
            free(filename);
            return 0;
        }
    }
    else if(type == 0){ // 后续是管道命令
        int end_index = 0;
        bool option = false;
        char* filename;
        char* temp1 = split_str(nextCommandStr, len_next, 0, &end_index, &option);
        if(temp1 == NULL){
            printf("\033[31m读取文件名称失败\033[m\n");
            return 1;
        }
        // 解析文件名
        if(temp1[0] == '~'){
            char root[255];
            ReadRoot(temp1, root, sizeof(root));
            filename = strdup(root);
        }
        else{
            filename = strdup(temp1);
        }
        if(temp1 != NULL){
            free(temp1);
        }
        // 将命令转换成管道命令执行
        int len_pipe_command = len_cur + len_next - split_index + 1;
        char pipe_command[len_pipe_command];
        int len_temp = len_next - split_index + 1;
        char temp[len_temp];
        strncpy(temp, nextCommandStr + split_index - 1, len_temp - 1);
        temp[len_temp - 1] = '\0';

        strcpy(pipe_command, commandStr);
        strcat(pipe_command, temp);
        
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            free(filename);
            return 1;
        }
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("fork");
            free(filename);
            return 1;
        }
        // pid_1的逻辑
        if(child_pid == 0){
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[1]);
            // 把读取端传递给Pipe
            if(Pipe(pipe_command, len_cur + 1, pipe_fd[0]) == 0)
                exit(EXIT_SUCCESS);
            else exit(EXIT_FAILURE);
        }
        else{
            close(pipe_fd[0]);
            // 向管道写入文件内容
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                perror("fopen");
                close(pipe_fd[1]);
                free(filename);
                return 1;
            }

            char buffer[255];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(pipe_fd[1], buffer, bytesRead);
            }

            // 关闭写入端
            close(pipe_fd[1]);

            // 等待子进程结束
            waitpid(child_pid, NULL, 0);
            // 关闭文件
            fclose(file);
            free(filename);
            return 0;
        }
    }
    else if(type == 2 || type == 4){ // 后续是重定向输出命令
        int end_index = 0;
        bool option = false;
        char* filename;
        char* temp1 = split_str(nextCommandStr, len_next, 0, &end_index, &option);
        if(temp1 == NULL){
            printf("\033[31m读取文件名称失败\033[m\n");
            return 1;
        }
        // 解析文件名
        if(temp1[0] == '~'){
            char root[255];
            ReadRoot(temp1, root, sizeof(root));
            filename = strdup(root);
        }
        else{
            filename = strdup(temp1);
        }
        if(temp1 != NULL){
            free(temp1);
        }
        // 将命令转换成重定向命令执行
        int len_redirect_command = len_cur + len_next - split_index + 1;
        char redirect_command[len_redirect_command];
        int len_temp = len_next - split_index + 1;
        char temp[len_temp];
        strncpy(temp, nextCommandStr + split_index - 1, len_temp - 1);
        temp[len_temp - 1] = '\0';

        strcpy(redirect_command, commandStr);
        strcat(redirect_command, temp);

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            free(filename);
            return 1;
        }
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("fork");
            free(filename);
            return 1;
        }
        // pid_1的逻辑
        if(child_pid == 0){
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[1]);
            // 把读取端传递给重定向输出命令
            if(redirect(redirect_command, len_cur + 1, type, pipe_fd[0]) == 0)
                exit(EXIT_SUCCESS);
            else exit(EXIT_FAILURE);
        }
        else{
            close(pipe_fd[0]);
            // 向管道写入文件内容
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                perror("fopen");
                close(pipe_fd[1]);
                free(filename);
                return 1;
            }

            char buffer[255];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(pipe_fd[1], buffer, bytesRead);
            }

            // 关闭写入端
            close(pipe_fd[1]);

            // 等待子进程结束
            waitpid(child_pid, NULL, 0);
            // 关闭文件
            fclose(file);
            free(filename);
            return 0;
        }
    }
    printf("无法解析的命令: %s\n", nextCommandStr);
    return 1;
}
int many_in_redirect(const char* input, int split_index, int type, int PipeReadFd){return 0;}