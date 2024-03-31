#include "bin.h"

// 复制文件
int copyFile(const char *src, const char *dst) {  
    time_t currentTime = time(NULL);
    char temp[255];
    snprintf(temp, sizeof(temp), "temp_%ld", (long)currentTime);

    // 创建源文件和目标文件的文件描述符  
    int src_fd = open(src, O_RDONLY); 
    if (src_fd == -1) {  
        perror("open source file");  
        return 1;  
    }  
  
    int dst_fd = open(temp, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);  
    if (dst_fd == -1) {  
        perror("open destination file");  
        close(src_fd);  
        return 1;  
    }  
  
    // 读取源文件内容并写入目标文件  
    char buffer[4096];  
    ssize_t bytes_read;  
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {  
        ssize_t bytes_written = write(dst_fd, buffer, bytes_read);  
        if (bytes_written == -1) {  
            perror("write to destination file");  
            close(src_fd);  
            close(dst_fd);  
            return 1;  
        }  
    }  
    if (bytes_read == -1) {  
        perror("read from source file");  
        close(src_fd);  
        close(dst_fd);  
        return 1;  
    }  
  
    // 关闭文件描述符  
    close(src_fd);  
    close(dst_fd); 
    if(rename(temp, dst) != 0){
        remove(temp);
    }
    return 0;  
}  

int copy(const char*src, char*dst, size_t size_dst, const char*option){
    int judge = is_Dir_File(src);
    if( judge == -1){
        return 1;
    }
    else if(judge == 0){ // 文件
        char src_ext[12];
        int src_dir_index = getExtendedName(src, src_ext, sizeof(src_ext));
        char dst_ext[12];
        int dst_dir_index = getExtendedName(dst, dst_ext, sizeof(dst_ext));
        if(src_dir_index == 1 || dst_dir_index == 1){
            perror("\033[0;31mError: mv move getExtendedName\033[0m\n");
            return 1;
        }
        if(src_dir_index == -1){ // 文件无后缀
            if(option != NULL){
                if(strcmp(option, "-f") == 0){
                    if(access(dst, F_OK) == 0){
                        perror("\033[0;31m存在同名文件, 请检查\033[0m\n");
                        return 1;
                    }
                    char lastDir[255];
                    if(getLastDir(dst, lastDir, sizeof(lastDir)) == -1){
                        perror("\033[0;31m获取文件目录失败, 请检查\033[0m\n");
                        return 1;
                    }
                    DIR* dir = opendir(lastDir);
                    if(dir == NULL){ // 目录不存在
                        if(createDirectories(lastDir) == 1){
                            closedir(dir);
                            return 1;
                        }
                    }
                    closedir(dir);
                    if(copyFile(src, dst) == 1){
                        return 1;
                    }
                    return 0;
                }
            }
            DIR* dir = opendir(dst);
            if(dir == NULL){ // 目录不存在
                if(createDirectories(dst) == 1){
                    closedir(dir);
                    return 1;
                }
            }
            closedir(dir);
            if(getFileName(src, dst, size_dst) == 1){
                perror("\033[0;31mmv getFileName\033[0m\n");
                return 1;
            }
            if(copyFile(src, dst) == 1){
                return 1;
            }
            return 0;
        }
        else{ // 文件有后缀 
            if(dst_dir_index == -1){ // dst是文件夹
                DIR* dir = opendir(dst);
                if(dir == NULL){ // 目录不存在
                    if(createDirectories(dst) == 1){
                        closedir(dir);
                        return 1;
                    }
                }
                closedir(dir);
                if(getFileName(src, dst, size_dst) == 1){
                    perror("\033[0;31mmv getFileName\033[0m\n");
                    return 1;
                }
                if(copyFile(src, dst) == 1){
                    return 1;
                }
                return 0;
            }
            else{
                if(strcmp(src_ext, dst_ext) != 0){
                    perror("\033[0;31m文件的后缀格式不同, 请检查\033[0m\n");
                    return 1;
                }
                if(access(dst, F_OK) == 0){
                    perror("\033[0;31m存在同名文件, 请检查\033[0m\n");
                    return 1;
                }
                char lastDir[255];
                if(getLastDir(dst, lastDir, sizeof(lastDir)) == -1){
                    perror("\033[0;31m获取文件目录失败, 请检查\033[0m\n");
                    return 1;
                }
                DIR* dir = opendir(lastDir);
                if(dir == NULL){ // 目录不存在
                    if(createDirectories(lastDir) == 1){
                        closedir(dir);
                        return 1;
                    }
                }
                closedir(dir);
                if(copyFile(src, dst) == 1){
                    return 1;
                }
                return 0;
            }
        }
        return 0;

    }
    else{   // 文件夹
        DIR* dir = opendir(src);
        struct dirent* entry;
        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                char subsrc[size_dst];
                char subdst[size_dst];
                strcpy(subsrc, src);
                strcpy(subdst, dst);
                // 获取源文件或源文件夹的完整URL
                if(getFileName(entry->d_name, subsrc, size_dst) == 1){
                    perror("\033[0;31mmv getFileName\033[0m\n");
                    return 1;
                }
                // 如果是目录, subdst则应是相应的层级关系
                if(entry->d_type == DT_DIR){
                    if(getFileName(entry->d_name, subdst, size_dst) == 1){
                        perror("\033[0;31mmv getFileName\033[0m\n");
                        return 1;
                    }
                }
                // printf("move: %s     %s     to   %s\n", subsrc, entry->d_name, subdst);
                if(copy(subsrc, subdst, size_dst, NULL) == 1){ // 递归复制
                    return 1;
                }
            }
        }
        closedir(dir);
        return 0;
    }
}
int cp(const Command* command){
    if(command->num_params == -1 || command->num_params > 0){
        return 0;
    }
    if(command->params->num_values > 2 || command->params->num_values < 2){
        perror("\033[0;31m无效的参数\033[0m\n");
        return 1;
    }
    char src[255];
    char dst[255];
    if (command->params->value[0][0] == '~')
    {
        ReadRoot(command->params->value[0], src, sizeof(src));
    }
    else if(command->params->value[0][0] != '/'){
        int last = findRelative(command->params->value[0], src);
        getDir(last, src, sizeof(src));
    }
    if (command->params->value[1][0] == '~')
    {
        ReadRoot(command->params->value[1], dst, sizeof(dst));
    }
    else if(command->params->value[1][0] != '/'){
        int last = findRelative(command->params->value[1], dst);
        getDir(last, dst, sizeof(dst));
    }

    int judge = is_Dir_File(src);
    if(judge == -1){
        return 1;
    }
    else if(judge == 1){
        if(command->params->key != NULL){
            if(strcmp(command->params->key, "-r") != 0){
                printf("\033[0;31m复制失败, 将要复制的是一个文件夹: \033[0m%s\n\033[0;31m如果需要复制文件夹, 请添加-r参数避免错误复制\033[0m\n", src);
                return 1;
            }
        }
        else{
            printf("\033[0;31m复制失败, 将要复制的是一个文件夹: \033[0m%s\n\033[0;31m如果需要复制文件夹, 请添加-r参数避免错误复制\033[0m\n", src);
            return 1;
        }
    }
    if (copy(src, dst, sizeof(dst), command->params->key) == 1){
        perror("\033[0;31m复制失败\033[0m\n");
        return 1;
    }
    return 0;
}