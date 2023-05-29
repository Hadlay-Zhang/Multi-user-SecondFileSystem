#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <functional>
#include <map>
#define PORT 1234
#define BACKLOG 128

#include "../include/Interaction.h"
#include "../include/Kernel.h"

using std::endl;
using std::string;

// 定义的线程参数结构体
typedef struct {
    int connectfd;
    std::map<std::string, std::string> user2passwd;
} ThreadData;

// 用于switch结构
std::unordered_map<std::string, int> Api2Int = {
        {"", API_NULL},
        {" ", API_SPACE},
        {"help", API_HELP},
        {"cd", API_CD},
        {"ls", API_LS},
        {"touch", API_TOUCH},
        {"mkdir", API_MKDIR},
        {"rm", API_RM},
        {"open", API_OPEN},
        {"close", API_CLOSE},
        {"read", API_READ},
        {"write", API_WRITE},
        {"seek", API_SEEK},
        {"cat", API_CAT},
        {"cpin", API_CPIN},
        {"cpout", API_CPOUT},
        {"exit", API_EXIT}
};

bool Interaction::isNumber(const string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

std::string Interaction::printInitInfo()
{
    std::stringstream ss;
    ss << "====================================" << endl;
    ss << "\033[1;33m多用户二级文件系统 by 2054169 张智淋\033[0m" << endl;
    ss << "====================================" << endl;
    ss << "输入\033[1;33mhelp\033[0m查看所有函数信息与输入格式..." << endl;
    ss << "====================================" << endl;
    return ss.str();
};

void Interaction::printHelp(std::stringstream &input_s, int socketfd, std::string username)
{
    std::stringstream ss;
    string param1;
    input_s >> param1;
    if (param1 == "") {
        ss << "open [filename] [mode]                     " << '\n';
        ss << "close [fd]                                 " << '\n';
        ss << "read [fd] [length]                         " << '\n';
        ss << "write [fd] [contents]                      " << '\n';
        ss << "seek [fd] [offset] [whence]                " << '\n';
        ss << "touch [filename] [mode]                    " << '\n';
        ss << "rm [filename]                              " << '\n';
        ss << "ls                                         " << '\n';
        ss << "mkdir [dirname]                            " << '\n';
        ss << "cd [dirname]                               " << '\n';
        ss << "cat [filename]                             " << '\n';
        ss << "cpin [out_filename] [filename]             " << '\n';
        ss << "cpout [filename] [out_filename]            " << '\n';
        ss << "exit                                       " << '\n';
        ss << "输入 help [funcname] 查看函数具体说明.        " << '\n';
        send2client(socketfd, username, ss.str());
        return;
    };
    // help具体API
    switch (Api2Int[param1]) {
        case (API_LS):
            ss << "\033[1;33m显示当前目录下所有内容\033[0m" << endl;
            ss << "ls" << endl;
            break;
        case (API_CD):
            ss << "\033[1;33m改变当前工作目录\033[0m" << endl;
            ss << "cd [dirname]" << endl;
            ss << "(const char*) dirname: 路径" << endl;
            break;
        case (API_MKDIR):
            ss << "\033[1;33m创建文件夹\033[0m" << endl;
            ss << "mkdir [dirname]" << endl;
            ss << "(const char*) dirname: 文件夹名" << endl;
            break;
        case (API_RM):
            ss << "\033[1;33m删除文件(夹)\033[0m" << endl;
            ss << "rm [filename]" << endl;
            ss << "(const char*) filename: 要删除的文件(夹)名" << endl;
            break;
        case (API_CAT):
            ss << "\033[1;33m显示文件内容\033[0m" << endl;
            ss << "cat [filename]" << endl;
            ss << "(const char*) filename: 文件名" << endl;
            break;
        case (API_OPEN):
            ss << "\033[1;33m打开文件\033[0m" << endl;
            ss << "open [filename] [mode]" << endl;
            ss << "(const char*) filename: 要打开的文件名" << endl;
            ss << "(int) mode: 打开模式，默认0777" << endl;
            break;
        case (API_CLOSE):
            ss << "\033[1;33m关闭文件\033[0m" << endl;
            ss << "close [fd]" << endl;
            ss << "(int) fd: 要关闭的文件的文件描述符" << endl;
            break;
        case (API_READ):
            ss << "\033[1;33m读取文件\033[0m" << endl;
            ss << "read [fd] [length]" << endl;
            ss << "(int) fd: 要读取的文件的文件描述符" << endl;
            ss << "(int) length: 读取长度" << endl;
            break;
        case (API_WRITE):
            ss << "\033[1;33m写入文件\033[0m" << endl;
            ss << "write [fd] [contents]" << endl;
            ss << "(int) fd: 要写入的文件的文件描述符" << endl;
            ss << "(const char*) contents: 要写入的内容" << endl;
            break;
        case (API_SEEK):
            ss << "\033[1;33m移动文件指针\033[0m" << endl;
            ss << "seek [fd] [offset] [whence]" << endl;
            ss << "(int) fd: 文件的文件描述符" << endl;
            ss << "(int) offset: 文件指针移动偏移量" << endl;
            ss << "(int) whence: 0->从文件开头 1->从当前位置 2->从文件结尾" << endl;
            break;
        case (API_TOUCH):
            ss << "\033[1;33m创建文件\033[0m" << endl;
            ss << "touch [filename] [mode]" << endl;
            ss << "(const char*) filename: 要创建的文件的文件名" << endl;
            ss << "(int) mode: 文件模式，默认IRWXU" << endl;
            break;
        case (API_CPIN):
            ss << "\033[1;33m复制系统外的文件到当前目录\033[0m" << endl;
            ss << "cpin [out_filename] [filename]" << endl;
            ss << "(const char*) out_filename: 系统外部待被拷贝的文件名" << endl;
            ss << "(const char*) filename: 要复制进当前目录的新文件名" << endl;
            break;
        case (API_CPOUT):
            ss << "\033[1;33m复制系统内当前目录下的文件到系统外\033[0m" << endl;
            ss << "cpout [filename] [out_filename]" << endl;
            ss << "(const char*) filename: 系统内部待被拷贝的文件名" << endl;
            ss << "(const char*) out_filename: 要复制到系统外部的新文件名" << endl;
            break;
        case(API_EXIT):
            ss << "\033[1;33m退出程序\033[0m" << endl;
            ss << "exit" << endl;
            break;
        default:
            ss << "不存在此函数，请重新输入." << endl;
            break;
    }
    send2client(socketfd, username, ss.str());
}

void Interaction::Api_cd(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string param1;
    input_s >> param1;
    if (param1 == "")
    {
        return_s << "cd: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 调用
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    char dirname[300] = {0};
    strcpy(dirname, param1.c_str());
    u.u_dirp = dirname;
    u.u_arg[0] = (unsigned long long)(dirname);
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.ChDir();
};

void Interaction::Api_ls(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    string cur_path = u.u_curdir;
    unsigned int fd = Kernel::Instance().Sys_Open(cur_path, (File::FREAD));
    char buf[33] = {0};
    while (1)
    {
        if (Kernel::Instance().Sys_Read(fd, 32, 33, buf) == 0)
            break;
        else
        {
            DirectoryEntry *mm = (DirectoryEntry *)buf;
            if (mm->m_ino == 0)
                continue;
            return_s << mm->m_name << " ";
            memset(buf, 0, 32);
        }
    }
    Kernel::Instance().Sys_Close(fd);
    return_s << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_mkdir(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string path;
    input_s >> path;
    if (path == "")
    {
        return_s << "mkdir: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int ret = Kernel::Instance().Sys_Mkdir(path);
    return_s << "mkdir 返回值: " << ret << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_touch(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string filename;
    input_s >> filename;
    if (filename == "")
    {
        return_s << "touch: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    char c_filename[512];
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    u.u_ar0[0] = 0;
    u.u_arg[1] = Inode::IRWXU;
    u.u_dirp = c_filename;
    strcpy(c_filename, filename.c_str());
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.Creat();
};

void Interaction::Api_rm(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string filename;
    input_s >> filename;
    if (filename == "")
    {
        return_s << "rm: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    u.u_ar0[0] = 0;
    u.u_ar0[1] = 0;
    char c_filename[512];
    strcpy(c_filename, filename.c_str());
    u.u_dirp = c_filename;
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.UnLink();
};

void Interaction::Api_seek(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string fd, offset, whence;
    input_s >> fd >> offset >> whence;
    if (fd == "" || offset == "" || whence == "")
    {
        return_s << "seek: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(fd))
    {
        return_s << "seek: 参数fd错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 用于检查负数偏移
    bool flag_offset = static_cast<char>(offset[0]) == '-' && isNumber(offset.substr(1));
    if (!flag_offset && !isNumber(offset))
    {
        return_s << "seek: 参数offset错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(whence))
    {
        return_s << "seek: 参数whence错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int fd_int = atoi(fd.c_str());
    int offset_int = atoi(offset.c_str());
    int whence_int = atoi(whence.c_str());
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    u.u_arg[0] = fd_int;
    u.u_arg[1] = offset_int;
    u.u_arg[2] = whence_int;
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.Seek();
    return_s << "新的读写位置指针: " << u.u_ar0 << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_open(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string para1;
    string para2;
    input_s >> para1 >> para2;
    if (para1 == "")
    {
        return_s << "open: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    string filename = para1;
    if (!isNumber(para2))
    {
        return_s << "open: 参数mode错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int mode;
    if (para2 == "")
        mode = 0777;
    else {
        int tmp = atoi(para2.c_str());
        // 进制转换
        mode = tmp / 100 * 64 + tmp % 100 / 10 * 8 + tmp % 10;
    }
    // 调用
    unsigned int fd = Kernel::Instance().Sys_Open(filename, mode);
    // 打印结果
    return_s << "文件: " << filename << " 打开成功." << endl << "fd = " << fd << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_read(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string para1;
    string para2;
    input_s >> para1 >> para2;
    if (para1 == "" || para2 == "")
    {
        return_s << "read: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(para1))
    {
        return_s << "read: 参数fd错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(para2))
    {
        return_s << "read: 参数length错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int fd = atoi(para1.c_str());
    if (fd < 0)
    {
        return_s << "read: 参数fd应为正整数" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int size = atoi(para2.c_str());
    if (size <= 0 || size > 1024)
    {
        return_s << "read: size取值范围应为(0,1024]" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 调用 API
    char buf[1025];
    memset(buf, 0, sizeof(buf));
    int ret = Kernel::Instance().Sys_Read(fd, size, 1025, buf);
    // 结果返回
    return_s << "成功读入 " << ret << " 字节，内容为: " << endl
             << buf << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_write(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string para1 = "";
    string para2 = "";
    input_s >> para1 >> para2;
    if (para1 == "")
    {
        return_s << "write: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(para1))
    {
        return_s << "write: 参数fd错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int fd = atoi(para1.c_str());
    if (fd < 0)
    {
        return_s << "write: 参数fd应为正整数" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (para2.length() > 1024)
    {
        return_s << "write: 内容过长，不能超过1024字节" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    char buf[1025];
    memset(buf, 0, sizeof(buf));
    strcpy(buf, para2.c_str());
    int size = para2.length();
    // 调用 API
    int ret = Kernel::Instance().Sys_Write(fd, size, 1024, buf);
    // 打印结果
    return_s << "成功写入 " << ret << " 字节." << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_close(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string para1;
    input_s >> para1;
    if (para1 == "")
    {
        return_s << "close: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    if (!isNumber(para1))
    {
        return_s << "close: 参数fd错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    int fd = atoi(para1.c_str());
    if (fd < 0)
    {
        return_s << "close: 参数fd应为正整数" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 调用 API
    int ret = Kernel::Instance().Sys_Close(fd);
    return_s << "close 返回值: " << ret << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_cat(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string para1;
    input_s >> para1;
    if (para1 == "")
    {
        return_s << "cat: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    string filename = para1;
    // 打开文件
    unsigned int fd = Kernel::Instance().Sys_Open(filename, 0x1);
    if (fd < 0)
    {
        return_s << "[ERROR]: cat: 无法打开文件." << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 读取文件内容，每次读取256字节
    char buf[257];
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        int ret = Kernel::Instance().Sys_Read(fd, 256, 256, buf);
        if (ret <= 0)
            break;
        return_s << buf;
    }
    // 读取完毕，关闭文件
    Kernel::Instance().Sys_Close(fd);
    return_s << endl;
    send2client(socketfd, username, return_s.str());
};

void Interaction::Api_cpin(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string out_filepath;
    string filepath;
    input_s >> out_filepath >> filepath;
    if (out_filepath == "" || filepath == "")
    {
        return_s << "cpin: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 打开系统外部文件
    int ofd = open(out_filepath.c_str(), O_RDONLY); // 只读方式打开外部文件
    if (ofd < 0)
    {
        return_s << "[ERROR]: 系统外部文件" << out_filepath << " 打开失败." << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 创建内部文件
    Kernel::Instance().Sys_Creat(filepath, 0x1 | 0x2);
    int ifd = Kernel::Instance().Sys_Open(filepath, 0x1 | 0x2);
    // 系统内部当前目录文件打开失败
    if (ifd < 0)
    {
        close(ofd);
        return_s << "[ERROR]: 系统内部当前目录下文件: " << filepath << " 打开失败." << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 开始拷贝，一次 256 字节
    char buf[256];
    int read_count = 0;
    int write_count = 0;
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        int read_num = read(ofd, buf, 256);
        if (read_num <= 0)
            break;
        read_count += read_num;
        int write_num = Kernel::Instance().Sys_Write(ifd, read_num, 256, buf);
        if (write_num <= 0)
        {
            return_s << "[ERROR]: 写入系统内部当前目录下文件: " << filepath << "失败." << endl;
            break;
        }
        write_count += write_num;
    }
    return_s << "共读取字节：" << read_count << " 共写入字节：" << write_count << endl;
    close(ofd);
    Kernel::Instance().Sys_Close(ifd);
    send2client(socketfd, username, return_s.str());
}

void Interaction::Api_cpout(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username)
{
    string filepath;
    string out_filepath;
    input_s >> filepath >> out_filepath;
    if (filepath == "" || out_filepath == "")
    {
        return_s << "cpout: 参数个数错误" << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 创建系统外部文件
    int ofd = open(out_filepath.c_str(), O_WRONLY | O_TRUNC | O_CREAT);
    if (ofd < 0)
    {
        return_s << "[ERROR]: 系统外部文件: " << out_filepath << " 创建打开失败." << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 打开系统内部当前目录下文件
    int ifd = Kernel::Instance().Sys_Open(filepath, 0x1 | 0x2);
    if (ifd < 0)
    {
        close(ofd);
        return_s << "[ERROR]: 系统内部当前目录下文件: " << filepath << " 打开失败." << endl;
        send2client(socketfd, username, return_s.str());
        return;
    }
    // 开始拷贝，一次 256 字节
    char buf[256];
    int read_conut = 0;
    int write_count = 0;
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        int read_num = Kernel::Instance().Sys_Read(ifd, 256, 256, buf);
        if (read_num <= 0)
            break;
        read_conut += read_num;
        int write_num = write(ofd, buf, read_num);
        if (write_num <= 0)
        {
            return_s << "[ERROR]: 写入系统外部文件:" << out_filepath;
            break;
        }
        write_count += write_num;
    }
    return_s << "共读取字节：" << read_conut << " 共写入字节：" << write_count << endl;
    close(ofd);
    Kernel::Instance().Sys_Close(ifd);
    send2client(socketfd, username, return_s.str());
}

int Interaction::send2client(int fd, std::string username, std::string contents)
{
    int recBytes = send(fd, contents.c_str(), contents.length(), 0);
    // fflush(stdout);
    printf("[INFO]: 成功向用户 %s 发送 %d 字节.\n", username.c_str(), recBytes);
    return recBytes;
};

/* 接受用户输入循环 */
void* Interaction::MainLoop(void* arg) {
    ThreadData *threaddata = (ThreadData *)arg;
    int fd = threaddata->connectfd;
    std::map<std::string, std::string> user2passwd = threaddata->user2passwd;
    // int fd = *(static_cast<int*>(arg));
    char buf[1024];
    int recBytes;//server接收返回值
    // server向client发送登录提示
    char LoginStr[] = "请输入用户名：";
    recBytes = send(fd, LoginStr, sizeof(LoginStr), 0);
    // fflush(stdout);
    // std::cout << "[INFO]: 成功向用户发送输入用户名提示，字节数：" << recBytes << endl;
    printf("[INFO]: 成功向用户发送输入用户名提示，字节数：%d\n", recBytes);

    // server从client接收用户名
    memset(buf, 0, sizeof(buf));
    if ((recBytes = recv(fd, buf, 1024, 0)) == -1)
    {
        printf("[ERROR]: 接收用户名失败.\n");
        return (void *)nullptr;
    }

    // server端打印用户名
    string username = buf;
    printf("[INFO]: 输入的用户名为: %s.\n", username.c_str());

    // 检查花名册是否有该用户名
    if (!user2passwd.count(username)) {
        send2client(fd, username, "不存在该用户.\n");
        close(fd);
        return (void *)nullptr;
    }

    // server向client发送输入密码提示
    char PasswdStr[] = "请输入密码：";
    recBytes = send(fd, PasswdStr, sizeof(PasswdStr), 0);
    printf("[INFO]: 成功向用户发送输入密码提示，字节数：%d\n", recBytes);
    // server从client接收用户名
    memset(buf, 0, sizeof(buf));
    if ((recBytes = recv(fd, buf, 1024, 0)) == -1)
    {
        printf("[ERROR]: 接收密码失败.\n");
        return (void *)nullptr;
    }
    // server端检查密码是否与用户匹配
    std::string passwd = buf;
    if (user2passwd[username] != passwd) {
        send2client(fd, username, "密码不正确.\n");
        close(fd);
        return (void *)nullptr;
    }
    // 登录检查结束，开始服务

    // 初始化用户User结构和目录
    Kernel::Instance().GetUserManager().Login(username);

    char input[1024] = {0};     //用户输入的指令
    string tipswords;           //每行打印信息
    string api;                 //解析出来的文件函数API
    bool loop = true;

    // 循环读入用户指令
    while (loop) {
        api.clear();
        tipswords = "\033[1;32mMyFileSystem@" + username + "\033[0m:\033[1;34m" + Kernel::Instance().GetUser().u_curdir
                + "\033[0m$ ";

        memset(input, 0, sizeof(input));

        // server向client发送shell提示符
        recBytes = send(fd, tipswords.c_str(), tipswords.length(), 0);
        // fflush(stdout);
        if (recBytes > 0) {
            printf("[INFO]: 成功向用户 %s 发送shell提示符, 共 %d 字节.\n", username.c_str(), recBytes);
            // std::cout << "[INFO]: 成功向用户 " << username << " 发送shell提示符, 共 " << recBytes << " 字节." << endl;
        }
        // 若发送成功字节数小于0，说明用户掉线
        else {
            printf("[INFO]: 用户 %s 断开连接.\n", username.c_str());
            // std::cout << "[INFO]: 用户 " << username << " 断开连接." << endl;
            Kernel::Instance().GetUserManager().Logout();
            return (void *)nullptr;
        }
        // 读取用户输入的命令行
        if ((recBytes = recv(fd, input, 1024, 0)) == -1) {
            printf("[ERROR]: 读取用户输入命令失败.\n");
            Kernel::Instance().GetUserManager().Logout();
            return (void *)nullptr;
        }
        // 解析命令名称
        std::stringstream input_s(input);
        std::stringstream return_s;
        input_s >> api;
//        int debug = 0;
//        if (Api2Int[api]==API_NULL) {
//            debug = 1;
//        };
//        printf("[DEBUG]: %d\n", debug);
        switch (Api2Int[api]) {
            // 打印help信息
            case (API_HELP):
                printHelp(input_s, fd, username);
                break;
                // cd
            case (API_CD):
                Api_cd(input_s, return_s, fd, username);
                break;
            case (API_LS):
                Api_ls(input_s, return_s, fd, username);
                break;
            case (API_TOUCH):
                Api_touch(input_s, return_s, fd, username);
                break;
            case (API_MKDIR):
                Api_mkdir(input_s, return_s, fd, username);
                break;
            case (API_RM):
                Api_rm(input_s, return_s, fd, username);
                break;
            case (API_OPEN):
                Api_open(input_s, return_s, fd, username);
                break;
            case (API_CLOSE):
                Api_close(input_s, return_s, fd, username);
                break;
            case (API_READ):
                Api_read(input_s, return_s, fd, username);
                break;
            case (API_WRITE):
                Api_write(input_s, return_s, fd, username);
                break;
            case (API_SEEK):
                Api_seek(input_s, return_s, fd, username);
                break;
            case (API_CAT):
                Api_cat(input_s, return_s, fd, username);
                break;
            case (API_CPIN):
                Api_cpin(input_s, return_s, fd, username);
                break;
            case (API_CPOUT):
                Api_cpout(input_s, return_s, fd, username);
                break;
            case (API_EXIT):
                return_s << "用户 " << username << " 登出" << endl;
                send2client(fd, username, return_s.str());
                // Kernel::Instance().GetUserManager().Logout();
                loop = false;
                break;
            case (API_NULL):
                break;
            case (API_SPACE):
                break;
            default:
                std::stringstream instHelp;
                instHelp << api << ": 不存在该命令，输入help查看命令.\n";
                send2client(fd, username, instHelp.str());
                break;
        }
        if (loop == false) {
            break;
        }
    }
    printf("[INFO]: 结束 %s 的服务.\n\n", username.c_str());
    // GCC编译
    return nullptr;
}

void handle(int sig)
{
    // Nothing to do
}

void Interaction::Entrance()
{
    /* 1. 信号处理：通过sigaction函数注册一个信号处理函数来处理SIGPIPE信号 */
    struct sigaction action;
    action.sa_handler = handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);// 处理SIGPIPE信号，防止客户端意外断开导致服务器崩溃

    /* 2. 通过socket函数创建一个socket，用于监听客户端连接请求 */
    // 定义服务端地址和用户端地址结构体
    struct sockaddr_in server;
    struct sockaddr_in client;
    // 定义地址结构体的大小
    int socket_size = sizeof(struct sockaddr_in);
    // 定义监听和连接socket
    int listenfd, connectfd;
    // 通过socket函数创建socket，SOCK_STREAM代表TCP协议
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[ERROR]: 创建服务端socket失败.");
        exit(-1);
    }

    /* 3. 设置socket属性 */
    int opt = SO_REUSEADDR;
    // 设置socket属性，使得端口释放后能立刻被复用
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 清空服务端地址结构体
    bzero(&server, sizeof(server));
    // 设置使用的协议族为IPv4
    server.sin_family = AF_INET;
    // 设置端口
    server.sin_port = htons(PORT);
    // 设置服务端ip为本机任意ip
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 4. 绑定socket与指定ip和端口 */
    // 绑定地址和端口号
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[ERROR]: 绑定socket失败.");
        exit(-1);
    }

    /* 5. 设置socket为监听状态 */
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("[ERROR]: socket监听设置失败.\n");
        exit(-1);
    }

    /* 6. 初始化文件系统 */
    Kernel::Instance().Initialize(); // 初始化二级文件系统
    // 创建初始目录
    std::string bin = "bin";
    Kernel::Instance().Sys_Mkdir(bin);
    std::string etc = "etc";
    Kernel::Instance().Sys_Mkdir(etc);
    std::string home = "home";
    Kernel::Instance().Sys_Mkdir(home);
    std::string dev = "dev";
    Kernel::Instance().Sys_Mkdir(dev);
    printf("[INFO]: 创建初始目录完成.\n");
    // 创建花名册文件
    std::string etc_passwd = "/etc/passwd";
    char c_passwd[512];
    User &passwd_u = Kernel::Instance().GetUser();
    passwd_u.u_error = NOERROR;
    passwd_u.u_ar0[0] = 0;
    passwd_u.u_arg[1] = Inode::IRWXU;
    passwd_u.u_dirp = c_passwd;
    strcpy(c_passwd, etc_passwd.c_str());
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.Creat();
    // 将用户和密码写入花名册
    // (1) 先跳转到etc目录
    std::string etc_path = "/etc";
    char etc_path_c[512] = {0};
    strcpy(etc_path_c, etc_path.c_str());
    passwd_u.u_dirp = etc_path_c;
    passwd_u.u_arg[0] = (unsigned long long)(etc_path_c);
    filemanager.ChDir();
    // (2) 再打开passwd文件
    std::string passwd_filename = "passwd";
    int mode = 0777;
    unsigned int passwd_fd = Kernel::Instance().Sys_Open(passwd_filename, mode);
    // (3) 向花名册写入内容
    std::string passwd_contents = "user\t12345\ntest\t54321\n";
    std::map<std::string, std::string> user2passwd = {{"user", "12345"}, {"test", "54321"}};
    char passwd_buf[1025];
    memset(passwd_buf, 0, sizeof(passwd_buf));
    strcpy(passwd_buf, passwd_contents.c_str());
    int passwd_size = passwd_contents.length();
    Kernel::Instance().Sys_Write(passwd_fd, passwd_size, 1024, passwd_buf);
    // (4) 关闭花名册文件
    Kernel::Instance().Sys_Close(passwd_fd);
    printf("[INFO]: 二级文件系统准备就绪，等待用户接入...\n\n");

    /* 7. 通信循环，等待客户端连接并创建新线程 */
    while (1) {
        // accept函数等待客户端连接
        if ((connectfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&socket_size)) == -1) {
            perror("[WARNING]：socket accept失败.\n");
            continue;
        }
        printf("\n客户端: %s 接入成功.\n", inet_ntoa(client.sin_addr));
        // 定义一个线程号
        pthread_t thread;
        ThreadData threaddata;
        threaddata.connectfd = connectfd;
        threaddata.user2passwd = user2passwd;
        // 创建新线程
        pthread_create(&thread, NULL, &Interaction::ThreadStart, (void *)&threaddata);

    }
    close(listenfd); // 关闭监听socket
    return;
};

void* Interaction::ThreadStart(void* arg)
{
    Interaction* instance = static_cast<Interaction*>(arg);
    instance->MainLoop(arg);
    return NULL;
}
