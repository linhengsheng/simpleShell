#include "bin.h"
int getExtendedName(const char* url, char* temp, int len){
    int len_src = strlen(url);
    int index = -1;
    for(int i = len_src - 1;i >= 0;i--){
        if(url[i] == '.'){
            index = i;
            break;
        }
    }
    // 无后缀
    if(index == -1){
        temp[0] = '\0';
        return -1;
    }
    // 错误
    if(len_src - index > len){
        temp[0] = '\0';
        return 1;
    }
    // 有后缀
    for(int i = 0;i < len_src - index;i++){
        temp[i] = url[index + i + 1];
    }
    temp[len_src - index] = '\0';
    return 0;
}
// 获取上级目录
int getLastDir(const char* path, char* result, int len){
    int len_src = strlen(path);
    int index = -1;
    for(int i = len_src; i >= 0; i--){
        if(path[i] == '/'){
            index = i;
            break;
        }
    }
    // 没有上级目录
    if(index == -1){
        result[0] = '\0';
        return -1;
    }
    // 复制上级目录
    for(int i = 0; i < index; i++){
        result[i] = path[i];
    }
    result[index] = 0;
    return 0;
}
// 把src的文件名称拼接到dst目录下
int getFileName(const char*src, char* dst, int len){
    int len_src = strlen(src);
    int index = -1;
    for(int i = len_src;i >= 0; i--){
        if(src[i] == '/'){
            index = i;
            break;
        }
    }
    int len_dst = strlen(dst);
    // dst过小
    if(len_dst + len_src - index > len){
        return 1;
    }
    if(index == -1){ // src是一个单纯的文件名
        dst[len_dst] = '/';
        index = 0;
        for(int i = 0; i < len_src - index; i++){
            dst[i + len_dst + 1] = src[i + index];
        }
        dst[len_src - index + len_dst + 1] = '\0';
        return 0;
    }
    // src是由目录连接的文件URL
    for(int i = 0; i < len_src - index; i++){
        dst[i + len_dst] = src[i + index];
    }
    dst[len_src - index + len_dst] = '\0';
    return 0;
}
int CreateDirectory(const char* path){
    if (mkdir(path, 0777) == 0) {
        return 0; // 目录创建成功
    }else {
        printf("\033[0;31mFailed to create directory: \033[0m%s\n", path);
        return 1; // 目录创建失败
    }
}
int createDirectories(const char *path) {
    DIR* dir = opendir(path);
    if(dir == NULL){ // 目录不存在
        // 获取上级目录
        char lastDir[255];
        if(getLastDir(path, lastDir, sizeof(lastDir)) == -1){
            perror("\033[0;31mError: mv getLastDir\033[0m\n");
            return 1;
        }
        if(createDirectories(lastDir) == 1){
            return 1;
        }
        CreateDirectory(path);
        return 0;
    }
    return 0;
}
int is_Dir_File(const char* path){
    struct stat pathStat;
    
    // 获取文件/文件夹信息
    if (stat(path, &pathStat) != 0) {
        perror("\033[0;31mmv: is_Dir_File Failed to get file status\033[0m\n");
        return -1; // 返回-1表示操作失败
    }

    // 判断是否是文件夹
    if (S_ISDIR(pathStat.st_mode)) {
        return 1; // 返回1表示是文件夹
    } else {
        return 0; // 返回0表示是文件
    }
}
int move(const char*src, char*dst, size_t size_dst, const char* option){
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
                    rename(src, dst);
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
            if(access(dst, F_OK) == 0){
                perror("\033[0;31m存在同名文件, 请检查\033[0m\n");
                return 1;
            }
            rename(src, dst);
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
                // printf("dst: %s\n", dst);
                if(getFileName(src, dst, size_dst) == 1){
                    perror("\033[0;31mmv getFileName\033[0m\n");
                    return 1;
                }
                // printf("src: %s dst: %s\n", src, dst);
                if(access(dst, F_OK) == 0){
                    perror("\033[0;31m存在同名文件, 请检查\033[0m\n");
                    return 1;
                }
                rename(src, dst);
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
                rename(src, dst);
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
                if(move(subsrc, subdst, size_dst, NULL) == 1){ // 递归移动
                    return 1;
                }
            }
        }
        closedir(dir);
        // rmdir不会删除目录不为空的目录，因此移动到同名文件下不会发生错误
        if(rmdir(src) == -1){
            perror("rmdir");
            return 1;
        }
        return 0;
    }
}

int mv(const Command* command){
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
                printf("\033[0;31m移动失败, 将要移动的是一个文件夹: \033[0m%s\n\033[0;31m如果需要移动文件夹, 请添加-r参数避免错误移动\033[0m\n", src);
                return 1;
            }
        }
        else{
            printf("\033[0;31m移动失败, 将要移动的是一个文件夹: \033[0m%s\n\033[0;31m如果需要移动文件夹, 请添加-r参数避免错误移动\033[0m\n", src);
            return 1;
        }
    }

    if (move(src, dst, sizeof(dst), command->params->key) == 1){
        perror("\033[0;31m移动失败\033[0m\n");
        return 1;
    }
    return 0;
}