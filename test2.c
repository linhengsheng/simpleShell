#include "bin.h"

int getLastDir(const char* path, char* result, int len);
int getFileName(const char*src, char* dst, int len);
int CreateDirectory(const char* path);
int createDirectories(const char *path);
int main(){
    char t[255] = "/home/yueyueyue/todel/todel";
    // while(true){
    //     char r[100] = "/home";
    //     scanf("%s", t);
    //     printf("%d\n", getFileName(t, r, sizeof(r)));
    //     printf("%s\n%ld\n", r, strlen(r));
    // }
    createDirectories(t);
    return 0;
}