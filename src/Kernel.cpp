#include <string.h>

#include "../include/Kernel.h"

//全局单体实例
Kernel Kernel::instance;
DiskFileManager g_DiskFileManager;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
FileManager g_FileManager;
User g_User;
UserManager g_UserManager;


Kernel::Kernel()
{
    // nothing to do here
}

Kernel::~Kernel()
{
    // nothing to do here
}

// ======================= 获取成员实例对象函数 ======================= //
DiskFileManager &Kernel::GetDiskFileManager()
{
    return *(this->m_DiskFileManager);
}

BufferManager &Kernel::GetBufferManager()
{
    return *(this->m_BufferManager);
}

FileSystem &Kernel::GetFileSystem()
{
    return *(this->m_FileSystem);
}

FileManager &Kernel::GetFileManager()
{
    return *(this->m_FileManager);
}

User &Kernel::GetUser()
{
    return *(this->m_UserManager->GetUser());
}

UserManager& Kernel::GetUserManager()
{
    return *(this->m_UserManager);
}
// ======================= 获取成员实例对象函数 END ======================= //

// ======================= 获取Kernel static对象 ======================= //
Kernel &Kernel::Instance()
{
    return Kernel::instance;
}
// ======================= 获取Kernel static对象 END ======================= //

// ======================= 初始化相关函数 ======================= //
void Kernel::InitDiskFileManager()
{
    this->m_DiskFileManager = &g_DiskFileManager;
    this->m_DiskFileManager->Initialize();
}

void Kernel::InitFileSystem()
{
    printf("[INFO]: 开始初始化FileSystem...\n");
    this->m_FileSystem = &g_FileSystem;
    this->m_FileSystem->Initialize();
    this->m_FileManager = &g_FileManager;
    this->m_FileManager->Initialize();
    printf("[INFO]: FileSystem初始化完成.\n");
}

void Kernel::InitBuffer()
{
    printf("[INFO]: 开始初始化Buffer...\n");
    this->m_BufferManager = &g_BufferManager;
    this->m_BufferManager->Initialize();
    printf("[INFO]: Buffer初始化完成.\n");
}

void Kernel::InitUser()
{
    printf("[INFO]: 开始初始化User...\n");
    this->m_User = &g_User;
    this->m_UserManager = &g_UserManager;
    printf("[INFO]: User初始化完成.\n");
}
// ======================= 初始化相关函数 END ======================= //

// ======================= 文件系统初始化 ======================= //
void Kernel::Initialize()
{
    printf("[INFO]: 开始初始化Kernel...\n");
    this->InitBuffer();
    this->InitDiskFileManager();
    this->InitFileSystem();
    this->InitUser();

    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
    filemanager.rootDirInode->i_flag &= (~Inode::ILOCK);
    // 多用户new：根目录Inode解锁
    pthread_mutex_unlock(&filemanager.rootDirInode->mutex_inode);

    Kernel::Instance().GetFileSystem().LoadSuperBlock();
    User &us = Kernel::Instance().GetUser();
    us.u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
    us.u_cdir->i_flag &= (~Inode::ILOCK);
    // 多用户new：用户家目录Inode解锁
    pthread_mutex_unlock(&us.u_cdir->mutex_inode);
    strcpy(us.u_curdir, "/");

    printf("[INFO]: 二级文件系统 Kernel 初始化完毕.\n");
}
// ======================= 文件系统初始化 END ======================= //

// ======================= 退出文件系统 ======================= //
void Kernel::Quit()
{
    this->m_BufferManager->Bflush();
    this->m_FileManager->m_InodeTable->UpdateInodeTable();
    this->m_FileSystem->Update();
    this->m_DiskFileManager->Quit();
}
// ======================= 退出文件系统 END ======================= //

// ======================= 所实现的二级文件系统相关API ======================= //
unsigned int Kernel::Sys_Open(std::string &fpath, int mode)
{
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();
    char path[256];
    strcpy(path, fpath.c_str());
    u.u_dirp = path;
    // u.u_arg[0] = (int)path;
    u.u_arg[1] = mode;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Open();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Close(unsigned int fd)
{
    User &u = Kernel::Instance().GetUser();
    u.u_arg[0] = fd;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Close();

    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Mkdir(std::string &fpath)
{
    int default_mode = 040755;
    User &u = Kernel::Instance().GetUser();
    u.u_error = NOERROR;
    char filename_char[300] = {0};
    strcpy(filename_char, fpath.c_str());
    u.u_dirp = filename_char;
    u.u_arg[1] = default_mode;
    u.u_arg[2] = 0;
    FileManager &fimanag = Kernel::Instance().GetFileManager();
    fimanag.MkNod();
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Creat(std::string &fpath, int mode)
{
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();
    char path[256];
    strcpy(path, fpath.c_str());
    u.u_dirp = path;
    u.u_arg[0] = (long long)&path;
    u.u_arg[1] = mode;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Creat();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Delete(std::string &fpath)
{
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();
    char path[256];
    strcpy(path, fpath.c_str());
    u.u_dirp = path;
    u.u_arg[0] = (long long)&path;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.UnLink();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Read(unsigned int fd, size_t size, size_t nmemb, void *ptr)
{
    if (size > nmemb)
        return -1;
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();

    u.u_arg[0] = fd;
    u.u_arg[1] = (long long)ptr;
    u.u_arg[2] = size;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Read();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Write(unsigned int fd, size_t size, size_t nmemb, void *ptr)
{
    if (size > nmemb)
        return -1;
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();

    u.u_arg[0] = fd;
    u.u_arg[1] = (unsigned long long)ptr;
    u.u_arg[2] = size;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Write();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}

int Kernel::Sys_Seek(unsigned int fd, long int offset, int whence)
{
    // 模仿系统调用，将参数放入user结构中
    User &u = Kernel::Instance().GetUser();

    u.u_arg[0] = fd;
    u.u_arg[1] = offset;
    u.u_arg[2] = whence;

    FileManager &fileMgr = Kernel::Instance().GetFileManager();
    fileMgr.Seek();

    // 从user结构取出返回值
    return u.u_ar0[User::EAX];
}
// ======================= 所实现的二级文件系统相关API END ======================= //