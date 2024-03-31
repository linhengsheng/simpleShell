#include "bin.h"

#define PATH_LEN 256

void ls_dir(const char* path, const char* option){
    DIR *dir = NULL;
    struct dirent *entry = NULL;

    struct stat file_stat;
    if ((dir = opendir(path)) == NULL) {
        printf("\033[0;31m不是有效的目录: \033[0m%s\n", path);
        return;
    }
    char cwd[1024];
    getcwd(cwd,sizeof(cwd));
    chdir(path);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if(option == NULL){
            if (entry->d_name[0] == '.'){
                continue;
            }
        }
        else{
            if(strcmp(option, "-a") != 0){
                if (entry->d_name[0] == '.'){
                    continue;
                }
            }
        }
        // 获取文件信息
        if (stat(entry->d_name, &file_stat) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }
         if (S_ISREG(file_stat.st_mode)) {
            printf("%s  ", entry->d_name);
        } else if (S_ISDIR(file_stat.st_mode)) 
            printf("\033[0;34m%s  \033[0m", entry->d_name);
    }
    printf("\n");
    chdir(cwd);
    closedir(dir);
}

int ls(const Command *command) {
    if(command->num_params == -1){
        ls_dir(".", NULL);
    }
    if(command->num_params > 0){
        printf("\033[0;31m不支持的参数: \033[0m\n");
        for(int i=0; i<=command->num_params;i++){
            printf("%s ", command->params[i].key);
        }
        printf("\n");
        return 1;
    }
    DIR* dir;
    if (command->num_params == 0){
        if(command->params->value[0] == NULL){
            ls_dir(".", command->params->key);
            return 0;
        }
        if(command->params->value[0][0] == '~'){
            char root[255];
            ReadRoot(command->params->value[0], root, sizeof(root));
            if((dir = opendir(root)) == NULL){
                printf("\033[0;31m%s不是有效的目录\n\033[0m", root);
                return 1;
            }
            ls_dir(root, command->params->key);
            return 0;
        }
        else
            ls_dir(command->params->value[0], command->params->key);
            return 0;
    }
    return 0;
}