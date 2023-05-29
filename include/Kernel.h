#ifndef KERNEL_H
#define KERNEL_H

#include <string>

#include "DiskFileManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "User.h"
#include "UserManager.h"

class Kernel
{
public:
    Kernel();
    ~Kernel();

    /* 获取成员实例对象函数 */
    DiskFileManager& GetDiskFileManager();
    BufferManager& GetBufferManager();
    FileSystem& GetFileSystem();
    FileManager& GetFileManager();
    User& GetUser();
    UserManager& GetUserManager();

    // 获取Kernel static对象
    static Kernel& Instance();

    /* 初始化相关函数 */
    void InitDiskFileManager();
    void InitFileSystem();
    void InitBuffer();
    void InitUser();
    // 文件系统初始化
    void Initialize();
    // 退出文件系统
    void Quit();

    /* 所实现的二级文件系统相关API */
    unsigned int Sys_Open(std::string& fpath, int mode=File::FWRITE);
    int Sys_Close(unsigned int fd);
    int Sys_Mkdir(std::string& fpath);
    int Sys_Creat(std::string& fpath, int mode=File::FWRITE);
    int Sys_Delete(std::string& fpath);
    int Sys_Read(unsigned int fd, size_t size, size_t nmemb, void* ptr);
    int Sys_Write(unsigned int fd, size_t size, size_t nmemb, void* ptr);
    int Sys_Seek(unsigned int fd, long int offset, int whence); // whence: 0:文件开头 1:当前位置 2:文件结尾

private:
    static Kernel instance; // 单体实例
    /* 成员实例对象指针 */
    DiskFileManager* m_DiskFileManager;
    BufferManager* m_BufferManager;
    FileSystem* m_FileSystem;
    FileManager* m_FileManager;
    User* m_User;
    UserManager* m_UserManager;
};

#endif