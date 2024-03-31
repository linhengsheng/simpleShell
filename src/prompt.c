#include "prompt.h"

#define HOSTNAME_MAX 35
#define PATHMAX 255
#ifndef CHUNK_SIZE
#define CHUNK_SIZE 10  // 定义每次分配内存的块大小
#endif

int term_columns;

void handleResize(int signal) {
    (void) signal;  // 防止未使用的参数警告
    struct winsize ws;

    // 获取新的终端窗口大小
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return;
    }

    term_columns = ws.ws_col;
}
void getCursorPosition(unsigned int*row, unsigned int*col) {
    char buf[30];
    write(STDOUT_FILENO, "\033[6n", 4);  // 发送获取光标位置的ANSI Escape Code
    read(STDIN_FILENO, buf, sizeof(buf));
    sscanf(buf, "\033[%u;%uR", row, col);
}

// 函数用于恢复终端
void disableRawMode() {
    // tcgetattr获取当前终端设置
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    // 修改 c_lflag 标志位，将 ICANON 和 ECHO 位设置，以恢复规范模式和回显
    term.c_lflag |= (ICANON | ECHO);
    // 应用修改后的终端
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

// 函数用于启用终端原始模式，禁用行缓冲和回显
void enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    // 修改 c_lflag 标志位，将 ICANON（规范模式）和 ECHO（回显）位清除，以禁用规范模式和回显
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}
// 可能需要重新分配buffer，因此需要传二级指针，否则无法重新分配buffer
void insertChar(char** buffer, int position, char ch, int size, int* capacity) {
    // 如果当前字符串已满，需要扩展内存
    if (size >= *capacity) {
        *capacity += CHUNK_SIZE; // 扩展
        // printf("size: %d, cap: %d\n", size, *capacity);
        char* temp = (char*)realloc((*buffer), *capacity * sizeof(char));

        if (temp == NULL) {
            printf("内存分配失败\n");
            free((*buffer));
            return;
        }

        (*buffer) = temp;
    }

    // 将位置 position 及其后的字符向右移动一位
    for (int i = size; i > position; --i) {
        (*buffer)[i] = (*buffer)[i - 1];
    }

    // 在位置 position 插入新字符
    (*buffer)[position] = ch;
}
// 删除position处的字符
void delChar(char** buffer, int position, int size, int* capacity){
    if(10<size && size<*capacity-CHUNK_SIZE){
        *capacity -= CHUNK_SIZE;
        char* temp = (char*)realloc((*buffer), *capacity * sizeof(char));
        if (temp == NULL) {
            printf("内存分配失败\n");
            free((*buffer));
            return;
        }
        (*buffer) = temp;
    }
    for (int i = position; i < size; ++i) {
        (*buffer)[i] = (*buffer)[i + 1];
    }
}
int init_prompt(int* len){
    // 获取用户名称
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    bool root = false;
    if (pw == NULL) {
        perror("getpwuid");
        return 1;
    }
    // 获取主机名
    char hostname[HOSTNAME_MAX];
    
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        return 1;
    }

    char cwd[PATHMAX];
    // 获取当前工作目录
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return 1;
    }
    int home_len = 6 + strlen(pw->pw_name);
    char home[home_len+1];
    strcpy(home, "/home/");
    strcat(home, pw->pw_name);

    // 根据是否在用户目录下选择不同的提示语句
    char substr[home_len+1];
    int length = strlen(cwd);
    if(length>=home_len){
        strncpy(substr, cwd, home_len);
        if(strncmp(substr, home, home_len)==0){
            char cwd_[length-home_len+1];
            strcpy(cwd_,cwd+home_len);
            printf("\033[32m[shell]%s@%s\033[0m:\033[34m~%s\033[0m$ ", pw->pw_name, hostname, cwd_);
            root = true;
            *len = 12 + strlen(pw->pw_name) + strlen(hostname) + strlen(cwd_);
        }
    }
    if(root == false){
        printf("\033[32m[shell]%s@%s\033[0m:\033[34m%s\033[0m$ ", pw->pw_name, hostname, cwd);
        *len = 11 + strlen(pw->pw_name) + strlen(hostname) + strlen(cwd);
    }
    return 0;
}
char* getDynamicString(int *s, const int len_prompt) {
    // 注册信号处理函数
    if (signal(SIGWINCH, handleResize) == SIG_ERR) {
        perror("signal");
        return NULL;
    }

    // 获取初始终端窗口大小
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return NULL;
    }

    term_columns = ws.ws_col;
    // 光标位置
    int row, col;
    // 获取相应的字符串
    int indexHistory = historyCommands.count;
    int capacity = CHUNK_SIZE;  // 初始分配的内存容量
    int size = 0;  // 当前字符串的长度
    int position = 0;   //光标位置
    char* buffer = (char*)malloc(capacity * sizeof(char));  // 分配初始内存

    if (buffer == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    int ch;
    enableRawMode(); 
    /*
    position表示光标位置, (position+len_prompt) % term_columns = 0时表示字符到达右边界
    // 清除所有内容
    回到原始光标处:   row = (position + len_prompt) / term_columns;
                    while(row > 0){
                        printf("\033[F");
                        row--;
                    }
                    printf("\r\033[%dC", len_prompt);
                    printf("\033[J");
    insert: if position < size: printf(" \b");else: do nothing
    将光标左移n格:    
                    if(position < size){
                        int n = size - position;
                        position = size;
                        if((size + len_prompt) % term_columns == 0){
                            n--;
                            position--;
                        }
                        while(n>0){
                            col = (position + len_prompt) % term_columns;
                            if(col == 0){
                                printf("\033[F");
                                printf("\033[%dC", term_columns);
                            }
                            else{
                                printf("\033[D"); // 移动光标左侧
                            }
                            position--;
                            n--;
                        }
                    }
    */
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if(buffer==NULL){
            return NULL;
        }
        switch(ch){
            case '\033': // 捕获 Escape 字符
                // 判断后续字符是否表示箭头键
                getchar(); // 消耗 [
                char arrow = getchar(); // 获取表示箭头键的字符
                if (arrow == 'D') { // 表示左箭头
                    if (position > 0) {
                        col = (position + len_prompt)%term_columns;
                        if(col == 0){
                            printf("\033[F");
                            printf("\033[%dC", term_columns);
                        }
                        else{
                            printf("\033[D"); // 移动光标左侧
                        }
                        position--;
                    }
                }
                else if(arrow == 'C'){ // 右箭头
                    if(position<size){
                        col = (position + len_prompt)%term_columns;
                        if(col == term_columns-1){
                            printf("\033[E");
                        }
                        else{
                            printf("\033[C"); // 移动光标右侧
                        }
                        position++;
                    }
                }
                else if(arrow == 'A'){ // 上箭头
                    if(indexHistory>0){
                        // 清除所有内容
                        row = (position + len_prompt) / term_columns;
                        while(row > 0){
                            printf("\033[F");
                            row--;
                        }
                        printf("\r\033[%dC", len_prompt);
                        printf("\033[J");
                        free(buffer);
                        capacity = CHUNK_SIZE;
                        buffer = (char*)malloc(capacity * sizeof(char));
                        size = 0;
                        position = 0;
                        char* cmd = historyCommands.commands[--indexHistory];
                        int lencmd = strlen(cmd);
                        for(int i = 0; i < lencmd;i++){
                            insertChar(&buffer, position, cmd[i], size, &capacity);
                            position++;
                            size++;
                        }
                        for(int i=0; i<size; i++){
                            printf("%c", buffer[i]);
                        }
                        if ((position+len_prompt) % term_columns == 0)printf(" \b");
                        fflush(stdout);
                        break;
                    }
                }
                else if(arrow == 'B'){
                    if(indexHistory < historyCommands.count - 1){ // 下箭头
                        row = (position + len_prompt) / term_columns;
                        while(row > 0){
                            printf("\033[F");
                            row--;
                        }
                        printf("\r\033[%dC", len_prompt);
                        printf("\033[J");

                        free(buffer);
                        capacity = CHUNK_SIZE;
                        buffer = (char*)malloc(capacity * sizeof(char));
                        size = 0;
                        position = 0;
                        char* cmd = historyCommands.commands[++indexHistory];
                        int lencmd = strlen(cmd);
                        for(int i = 0; i < lencmd;i++){
                            insertChar(&buffer, position, cmd[i], size, &capacity);
                            position++;
                            size++;
                        }
                        for(int i=0; i<size; i++){
                            printf("%c", buffer[i]);
                        }
                        if ((position+len_prompt) % term_columns == 0)printf(" \b");
                        fflush(stdout); 
                        break;
                    }
                    else if(indexHistory == historyCommands.count - 1 ){
                        indexHistory++;
                        // 清除所有内容
                        row = (position + len_prompt) / term_columns;
                        while(row > 0){
                            printf("\033[F");
                            row--;
                        }
                        printf("\r\033[%dC", len_prompt);
                        printf("\033[J");
                        free(buffer);
                        capacity = CHUNK_SIZE;
                        buffer = (char*)malloc(capacity * sizeof(char));
                        size = 0;
                        position = 0;
                        fflush(stdout);
                        break;
                    }
                }
                else if(arrow == '3'){
                    char signo = getchar();
                    if(signo == '~'){ //  DEL键
                        if(size>position){
                            printf("\033[J"); // 清除光标后面的所有字符
                            delChar(&buffer, position, size, &capacity);
                            size--;
                            // 重新打印
                            for(int i=position; i<size; i++)
                                printf("%c", buffer[i]);
                            // 将光标左移n格
                            if(position < size){
                                int n = size - position;
                                position = size;
                                if((size + len_prompt) % term_columns == 0){
                                    n--;
                                    position--;
                                }
                                while(n>0){
                                    col = (position + len_prompt) % term_columns;
                                    if(col == 0){
                                        printf("\033[F");
                                        printf("\033[%dC", term_columns);
                                    }
                                    else{
                                        printf("\033[D"); // 移动光标左侧
                                    }
                                    position--;
                                    n--;
                                }
                            }
                            fflush(stdout);
                        }
                    }
                }
                break;
            case 127:   // BACKSPACE退格键
                if(position>0){
                    // 左移一位
                    col = (position + len_prompt)%term_columns;
                    if(col == 0){
                        printf("\033[F");
                        printf("\033[%dC", term_columns);
                    }
                    else{
                        printf("\033[D"); // 移动光标左侧
                    }
                    position--;
                    printf("\033[J");
                    delChar(&buffer, position, size, &capacity);
                    size--;
                    // 在终端中输出的字符串
                    for (int i = position; i < size; i++) {
                        printf("%c", buffer[i]);
                        
                    }
                    // 将光标左移n格
                    if(position < size){
                        int n = size - position;
                        position = size;
                        if((size + len_prompt) % term_columns == 0){
                            n--;
                            position--;
                        }
                        while(n>0){
                            col = (position + len_prompt) % term_columns;
                            if(col == 0){
                                printf("\033[F");
                                printf("\033[%dC", term_columns);
                            }
                            else{
                                printf("\033[D"); // 移动光标左侧
                            }
                            position--;
                            n--;
                        }
                    }
                    fflush(stdout);
                }
                break;
            default:
                printf("\033[J");
                fflush(stdout);
                // 在光标位置插入新字符
                insertChar(&buffer, position, (char)ch, size, &capacity);
                size++;
                // 在终端中输出插入后的字符串
                if((position+len_prompt) % term_columns == term_columns - 1){ // 当光标处于边界值的边缘时，先退一格再重新打印，避免终端显示的自动换行，防止出现奇怪的打印
                    printf("\b\033[K");
                    for(int i = position - 1; i<size; i++){
                        printf("%c", buffer[i]);
                    }
                }
                else{
                    for(int i = position; i<size; i++){
                        printf("%c", buffer[i]);
                    }
                }
                position++;
                // 为了可能出现的自动换行
                if((position+len_prompt) % term_columns == 0 && position == size){
                    printf(" \b");
                }
                // 将光标左移n格
                if(position < size){
                    int n = size - position;
                    position = size;
                    if((size + len_prompt) % term_columns == 0){
                        n--;
                        position--;
                    }
                    while(n>0){
                        col = (position + len_prompt) % term_columns;
                        if(col == 0){
                            printf("\033[F");
                            printf("\033[%dC", term_columns);
                        }
                        else{
                            printf("\033[D"); // 移动光标左侧
                        }
                        position--;
                        n--;
                    }
                }
                fflush(stdout);
                break;
        }
    }
    disableRawMode();
    printf("\n");
    // printf("%d\n", size);
    *s = size;
    if(size==0){
        free(buffer);
        return NULL;
    }
    // 添加字符串结尾的 null 字符, size可能等于CHUNK_SIZE的倍数，不能直接使用 buffer[size] = 0;否则发生越界访问
    // printf("%d\n", capacity);
    insertChar(&buffer, size, '\0', size, &capacity);
    // printf("%d\n", capacity);
    if(strcmp(buffer, "exit")==0){
        free(buffer);
        *s = -1;
        return NULL;
    }
    return buffer;
}