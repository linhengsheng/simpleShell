#include "bin.h"

// 递归输出指定目录的所有文件（递归用于遍历子目录的文件)
void tree_recursive(const char* path, const char* indent, const char* mode) {

    //初始化目录流指针
    DIR *dir;
    
    //表示目录中的一个子目录的信息
    struct dirent *entry;

    //表示目录的状态信息，比如大小、修改时间
    struct stat statbuf;

    //调用 opendir 函数尝试打开指定路径 path 的目录
    if ((dir = opendir(path)) == NULL) {
        return;
    }

    //将当前工作目录更改为指定的路径,并保存当前的工作目录信息
    char cwd[1024];
    getcwd(cwd,sizeof(cwd));
    chdir(path);


    //readdir(dir) 读取目录中的一个条目
    while ((entry = readdir(dir)) != NULL) {

        //获取entry->d_name所表示目录的状态信息，并将结果存储在statbuf结构体中
        lstat(entry->d_name, &statbuf);

        //.表示当前目录，..表示父目录，不是目录树下的一个具体目录，需过滤
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
            continue;
        }

        //根据输出模式过滤输出内容
        if((strcmp(mode,"-a"))==0)
        {
            //模仿linux的输出结构
            printf("%s├── %s\n", indent, entry->d_name);
        }
        else if((strcmp(mode,"-d"))==0)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                printf("%s├── %s\n", indent, entry->d_name);            
            }
        }
        

        //if包含的代码在判断当前目录项是否表示一个目录，若是，则进行递归处理
        if (S_ISDIR(statbuf.st_mode)) {
            
            char subdir_path[1024];
            
            //snprintf 函数将父目录的路径和当前目录项的名字组合起来,以"父目录/子目录"形式构建下一级目录
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", path, entry->d_name);

            char sub_indent[1024];
            
            //缩进方式：空格缩进或者"|"缩进，由主调函数决定。即第一次调用该函数传入的缩进方式决定了整个递归过程的缩进方式
            snprintf(sub_indent, sizeof(sub_indent), "%s%s", indent, (indent[strlen(indent) - 3] == '`') ? "   " : "│  ");
            tree_recursive(subdir_path, sub_indent, mode);
        }
    }
    //将当前工作目录更改为当前目录的父目录
    chdir(cwd);
    closedir(dir);
}

int tree(const Command* command) {

    //初始化路径为当前目录"." 缩进方式为""
    char indent[1024] = "";
    
    //没有输入地址
    if(command->num_params == -1){
        tree_recursive(".", indent, "-a");
        return 0;
    }

    if(command->num_params>0)
    {
        printf("\033[0;31m无效的选项: \033[0m");
        for(int i=0; i<=command->num_params;i++){
            printf("%s ", command->params[i].key);
        }
        printf("\n");
        return 1;
    }
    
    //没有指定输出格式,则默认是-a模式
    if((command->params->key)==NULL){
        //为command->params->key开辟空间
        command->params->key = (char*)malloc((strlen("-a") + 1) * sizeof(char));  
        strcpy(command->params->key,"-a");
    }
    DIR *dir;

    if(command->params->value[0][0] == '~'){
        char root[255];
        ReadRoot(command->params->value[0], root, sizeof(root));
        // printf("%s\n", root);
        // printf("%ld\n", strlen(root));
        if((dir = opendir(root)) == NULL){
            printf("\033[0;31m%s不是有效的目录\n\033[0m", root);
            return 1;
        }
        closedir(dir);
        tree_recursive(root, indent, command->params->key);
        return 0;
    }
    else if(command->params->value[0][0] == '/'){
        // printf("%s\n", command->params->value[0]);
        if((dir = opendir(command->params->value[0])) == NULL){
            printf("\033[0;31m%s不是有效的目录\n\033[0m\n", command->params->value[0]);
            return 1;
        }
        closedir(dir);
        tree_recursive(command->params->value[0], indent, command->params->key);
        return 0;
    }
    else{
        char dir_path[255];
        int last = findRelative(command->params->value[0], dir_path);
        getDir(last, dir_path, sizeof(dir_path));
        // printf("%s\n", dir_path);
        // printf("%ld\n", strlen(dir_path));
        if((dir = opendir(dir_path))== NULL){
            printf("\033[0;31m%s不是有效的目录\n\033[0m\n", dir_path);
            return 1;
        }
        closedir(dir);
        tree_recursive(dir_path, indent, command->params->key);
        return 0;
    }
}