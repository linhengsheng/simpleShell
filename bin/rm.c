#include "bin.h"

#define HOSTNAME_MAX 35
// 删除文件夹
void remove_dir(char *path)
{
    DIR *dir;
    struct dirent *dir_info;
    struct stat stat_buf;
    char file_path[1024] = {0};
    //调用lstat()函数获取指定路径的文件信息，并将结果保存在stat_buf结构体变量中
    lstat(path, &stat_buf);
    // 删除普通文件
    if (S_ISREG(stat_buf.st_mode) == 1) 
    {
        remove(path);
        return;
    }
    //删除文件夹
    else if (S_ISDIR(stat_buf.st_mode) == 1)
    {
        if ((dir = opendir(path)) == NULL)
        {   
            return;
        }
        while ((dir_info = readdir(dir)) != NULL) 
        {
            // 得到完整路径           
            // 跳过 .和.. 两个特殊目录
            if (strcmp(dir_info->d_name, ".") == 0 || strcmp(dir_info->d_name, "..") == 0)
                continue;
            snprintf(file_path, sizeof(file_path), "%s/%s", path, dir_info->d_name);

            // 若是文件夹，则递归到下一级目录中
            remove_dir(file_path);
        }
        closedir(dir);
    }
    // 删除被清空后的文件夹
    rmdir(path);
    return ;
}

int rm(const Command* command){

    struct stat file_stat;

    if(command->num_params == -1){
        printf("\x1b[31mNeed a file\x1b[0m\n");
        return 1;
    }
    if(command->num_params > 0){
        printf("\x1b[31m参数过多\x1b[0m\n");
        return 1;
    }
    if(command->params->num_values > 1){
        printf("\x1b[31m参数过多\x1b[0m\n");
        return 1;
    }
    // 解析文件
    int len_filename = strlen(command->params->value[0]);
    char filename[HOSTNAME_MAX + len_filename];
    filename[0] = '\0';
    if(command->params->value[0][0] == '~'){
        char root[255];
        ReadRoot(command->params->value[0], root, sizeof(root));
        strcpy(filename, root);
    }
    else{
        strcpy(filename, command->params->value[0]);
    }
    //获取指定路径的文件信息
    if(stat(filename, &file_stat)==-1){
        printf("\x1b[31mNo such file or directory\x1b[0m\n");
    }
    
    //没有指定输出格式是-r,且需要访问文件
    if((command->params->key)==NULL && S_ISDIR(file_stat.st_mode)){
        printf("\x1b[31m%s is a directory.\n", filename);
        printf("use -r to delete it.\x1b[0m\n");
        return 1;
    }

    if(command->params->key != NULL && S_ISDIR(file_stat.st_mode)){ // 递归删除文件夹
        if(strcmp(command->params->key,"-r")==0)
            remove_dir(filename);
        else{
            printf("\033[31mrm没有这样的参数: %s\n使用详情请使用help查看\033[0m\n", command->params->key);
        }
        return 0;
    }

    if(S_ISREG(file_stat.st_mode)==1){ // 文件删除
        printf("\x1b[31m");        
        remove(filename);
        printf("\x1b[0m");
        return 0;
    }
    

    return 0;
}