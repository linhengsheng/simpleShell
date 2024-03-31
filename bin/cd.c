#include "bin.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#endif
int findRelative(const char* str, char* result){
    int length = strlen(str);
    bool last = false;
    bool split = false;
    int lastindex = 0;
    int start = 0;
    for (int i = 0; i <= length; ++i) {
        if(str[i] != '.' && str[i] != '/')
        {
            start = i;
            break;
        }
        else{
            if(str[i] == '.'){
                if(split){
                    return -1;
                }
                if(last){
                    lastindex++;
                    split = true;
                }
                last = true;
            }
            else{
                last = false;
                split = false;
            }
        }
    }
    for(int i = 0;i <= length - start; i++){
        result[i] = str[i + start];
    }
    return lastindex;
}
void getDir(int lastindex, char* str, int len){
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == NULL) {
        perror("getcwd");
        return; // 获取当前工作目录失败
    }
    int lensrc = strlen(str);
    char temp[lensrc + 1];
    if(lensrc > 0){
        for(int i = 0;i <= lensrc;i++){
            temp[i] = str[i];
        }
    }
    
    int index = strlen(currentPath);
    while (lastindex > 0 && index > 0)
    {
        if(currentPath[index] == '/'){
            lastindex--;
            if(lastindex == 0)break;
        }
        index--;
    }
    if(index + lensrc >= len - 2)
    {
        return;
    }
    for(int i = 0; i < index; i++){
        str[i] = currentPath[i];
    }
    if(lensrc>0){
        str[index++] = '/';
        for(int i = 0;i <= lensrc;i++){
            str[i + index] = temp[i];
        }
        str[lensrc + index + 1] = '\0';
    }
    else{
        str[index] = '\0';
    }
}

void getRoot(char*temp, int len){
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    int lenusrname = strlen(pw->pw_name);
    int lenroot = 6 + lenusrname + 1;
    if(len < lenroot) return;
    strncpy(temp, "/home/", len - 1);
    temp[len - 1] = '\0';
    for(int i = 0;i < lenusrname;i++){
        temp[i + 6] = pw->pw_name[i];
    }
    temp[lenroot-1] = '\0';
}
void ReadRoot(const char*str, char*temp, int len){
    int lensrc = strlen(str);
    if(lensrc >= 2){
        if(str[1] != '/'){
            printf("不是有效的路径\n");
            return;
        }
    }
    char root[255];
    getRoot(root, sizeof(root));
    int len_root = strlen(root);
    // printf("len_root:%d  lensrc:%d  len:%d\n", len_root, lensrc, len);
    if(len_root + lensrc - 1 >= len){
        printf("路径过长\n");
        return;
    }
    if(lensrc > 2){
        for(int i = 0;i < lensrc - 1; i++){
            root[i + len_root] = str[i + 1];
        }
        root[lensrc + len_root - 1] = '\0';
    }
    else{
        root[len_root] = '\0';
    }
    strcpy(temp, root);

}
int cd(const Command* command){
    if(command->num_params>0) // 参数过多
    {
        printf("\033[0;31m无效的选项: \033[0m");
        for(int i=0; i<=command->num_params;i++){
            printf("%s ", command->params[i].key);
        }
        printf("\n");
        return 1;
    }
    else if(command->num_params==0){
        if(command->params->key!=NULL){ // cd没有option
            printf("\033[0;31m无效的选项: \033[0m%s\n", command->params->key);
            return 1;
        }
        if(command->params->num_values>1){ // 参数过多
            printf("\033[0;31m参数太多\033[0m\n");
            return 1;
        }
        if(command->params->value[0][0] == '~'){ // 解析～简化路径
            char root[255];
            ReadRoot(command->params->value[0], root, sizeof(root));
            if(chdir(root)!=0){
                printf("\033[0;31m无效的路径: \033[0mroot\n");
                return 1;
            }
        }
        else if(chdir(command->params->value[0])!=0){ // 解析普通路径
            printf("\033[0;31m无效的路径: \033[0m%s\n", command->params->value[0]);
            return 1;
        }
    }
    return 0;
}
