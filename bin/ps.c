#include "bin.h"
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int isInt(const char *str)
{
    while (*str)
    {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

int printProcessInfo(const char *pid)
{
    //将格式化的字符串写入path数组,path指向某个进程状态的信息
    char path[50];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);

    //read
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("\x1b[31mError opening file\x1b[0m");
        return -1;
    }

    //根据pron中stat的格式读入文件数据 
    //进程的名称
    char command[256];
    //进程的状态
    char state;
    //父进程ID 所在进程组的ID 会话ID 终端设备号 所在前台进程组的ID
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned int flags;
    //次要缺页中断次数 子进程次要缺页中断次数 进程缺页中断次数 子进程缺页中断次数 进程在用户态运行的时间 进程在内核态运行的时间 
    unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
    //所有子进程的用户态运行时间之和 所有子进程的内核态运行时间之和 静态优先级 线程数，定时器的值，启动时刻 虚拟内存大小 驻留内存大小
    long cutime, cstime, priority, nice, num_threads, itrealvalue, starttime, vsize, rss;

    //%*d表示忽略读取到的整数值
    fscanf(file, "%*d %255s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %ld %ld %ld",
           command, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags,
           &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime, &cutime, &cstime,
           &priority, &nice, &num_threads, &itrealvalue, &starttime, &vsize, &rss);
    
    //只输出运行中的进程
    if(state!='R'){
        return 0;
    }

    fclose(file);

    // 获取用户名
    char username[256];
    FILE *statusFile = fopen("/proc/self/status", "r");
    if (statusFile == NULL)
    {
        perror("\x1b[31mError opening status file\x1b[0m");
        return -1;
    }

    //逐行读取文件的内容:此path不同上面的，上面的path已经没用了，只是沿用其变量名而已
    while (fgets(path, sizeof(path), statusFile) != NULL)
    {   
        // 查找行中是否包含特定的字符串"Uid:"
        if (strstr(path, "Uid:") != NULL)
        {   
            // sscanf()函数根据指定的格式化字符串，从字符串中提取数据并存储到username变量中。%*s表示忽略读取到的字符串值,即把Uid：后的内容保存到username变量中
            sscanf(path, "%*s %s", username);
            break;
        }
    }
    fclose(statusFile);

    // CPU使用率
    double cputime = (double)(utime + stime) / sysconf(_SC_CLK_TCK);
    double current = (double)(clock() / CLOCKS_PER_SEC);
    double start = (double)starttime;
    double total = start - current;
    double cpu_use = cputime / total;

    // 获取物理内存大小，计算内存使用率
    // long mem = rss * 4   因为rss通常是指4KB大小的内存块数
    FILE *meminfo_file = fopen("/proc/meminfo", "r");
    if (meminfo_file == NULL)
    {
        perror("\x1b[31mError opening /proc/meminfo\x1b[0m");
        exit(EXIT_FAILURE);
    }
    unsigned long total_memory = 0;
    //读取系统的总内存大小
    while (1)
    {
        char line[256];
        //得到的行存储在line中
        //到达最终行
        if (fgets(line, sizeof(line), meminfo_file) == NULL)
        {
            break;
        }
        //找到了正确的系统总内存大小
        if (sscanf(line, "MemTotal: %lu kB", &total_memory) == 1)
        {
            break;
        }
    }
    fclose(meminfo_file);

    double mem_use = (double)(rss * 4) / (double)total_memory;

    //"USER", "PID", "%CPU", "%MEM", "VSZ", "RSS", "TTY", "STAT", "START", "TIME", "COMMAND"
    printf("%-10s %-5s %7.2f %-7.2f %-15lu %-15lu %-5d %-5c %-10.0f %-10.0f %-20s\n",
           username, pid, cpu_use, mem_use, vsize, rss, tty_nr, state, start, total, command);
    return 0;
}

int ps(const Command* command){
    // proc是一个特殊的目录，在Linux系统中保存了关于进程和系统状态的信息
    DIR *dir = opendir("/proc");
    if (dir == NULL)
    {
        perror("\x1b[31mError opening directory\x1b[0m\n");
        return -1;
    }

    // 进程信息的标签
    // 进程的用户、进程ID、CPU使用率、内存使用率、虚拟内存大小、物理内存（RSS）占用量、终端类型、进程状态、进程启动时间、进程消耗CPU时间、命令行字符串
    printf("%-10s %-5s   %-5s%-5s  %-15s %-15s %-5s %-5s %-10s %-10s %-20s\n",
           "USER", "PID", "CPU(%)", "MEM(%)", "VSZ(Bytes)", "RSS(Pages)", "TTY", "STAT", "START(s)", "TIME(s)", "COMMAND");

    
    struct dirent *entry;
    //遍历"/proc"中的文件
    while ((entry = readdir(dir)) != NULL)
    {
        // 目录名称是数字,确保只处理表示进程编号的文件
        if (isInt(entry->d_name))
        {
            // 函数返回值为-1，说明出现了错误
            if (printProcessInfo(entry->d_name) == -1)
                return -1;
        }
    }

    closedir(dir);
    return 1;
}   