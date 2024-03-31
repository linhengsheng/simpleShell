#include "bin.h"

#define HOSTNAME_MAX 35
int exe(const Command* command){
    int len_name = strlen(command->command_name);
    char command_name[HOSTNAME_MAX + len_name];
    command_name[0] = '\0';
    // 解析是否为文件名称
    if(command->command_name[0] == '~'){
        char root[255];
        ReadRoot(command->command_name, root, sizeof(root));
        strcpy(command_name, root);
    }
    else{
        strcpy(command_name, command->command_name);
    }
    // printf("%s\n", command_name);
    // printf("%ld\n", strlen(command_name));


    // 将所有的参数转化成二进制文件command->command_name的参数
    int num_param = 0;
    for(int i = 0; i <= command->num_params; i++){
        if(command->params[i].key != NULL){
            num_param++;
        }
        for(int j = 0;j < command->params[i].num_values;j++){
            if(command->params[i].value[j] != NULL){
                num_param++;
            }
        }
    }

    char** args = (char**)malloc((num_param + 1)*sizeof(char*));
    args[num_param] = NULL;

    int index = 0;
    for(int i = 0; i <= command->num_params; i++){
        if(command->params[i].key != NULL){
            args[index++] = strdup(command->params[i].key);
        }
        for(int j = 0;j < command->params[i].num_values;j++){
            if(command->params[i].value[j] != NULL){
                args[index++] = strdup(command->params[i].value[j]);
            }
        }
    }
    // printf("Arguments:\n");
    // for (int i = 0; args[i] != NULL; ++i) {
    //     printf("args[%d]: %s\n", i, args[i]);
    // }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("signal");
                exit(EXIT_FAILURE);
            }
        if (execve(command_name, args, NULL) == -1) {
            printf("Not an exe file or Known command: %s\n", command->command_name);
            exit(EXIT_FAILURE);
        }
    }
    else {
        // 等待子进程结束

        int status;
        pid_t terminated_child_pid = waitpid(child_pid, &status, 0);

        for(int i = 0;i < num_param; i++){
            if(args[i]!=NULL){
                free(args[i]);
            }
        }
        free(args);
        if (terminated_child_pid == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            return 0;
        }else if (WIFSIGNALED(status)){
            return 1;
        }else{
            return 1;
        }
    }
    return 0;
}