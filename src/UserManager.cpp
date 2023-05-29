#include "../include/Kernel.h"
#include "../include/UserManager.h"

UserManager::UserManager()
{
    // 清空puser数组
    for (int i = 0; i < USER_N; ++i)
        (p_users)[i] = NULL;
    pid2index.clear();
    // UserManager构造时，分配第一个给系统作为主进程
    pthread_t pid = pthread_self();
    p_users[0] = (User *)malloc(sizeof(User));
    pid2index[pid] = 0;
}

UserManager::~UserManager()
{
    for (int i = 0; i < USER_N; ++i)
    {
        if ((p_users)[i] != NULL)
            free((p_users)[i]);
    }
}

// 用户登录
bool UserManager::Login(std::string uname)
{
    // 获取本线程的id
    pthread_t pthread_id = pthread_self();
    // 检查该线程是否已登录
    if (pid2index.find(pthread_id) != pid2index.end())
    {
        printf("[ERROR]: 线程 %lu ，用户名: %s 重复登录.\n", pthread_id, uname.c_str());
        return false;
    }
    // 寻找空闲的pusers指针
    int i;
    for (i = 0; i < USER_N; ++i)
    {
        if (p_users[i] == NULL)
            break;
    }
    // 若在线用户数超过最大并发数
    if (i == USER_N)
    {
        printf("[ERROR]: 用户并发数量达到上限，无法登录.\n");
        return false;
    }
    // 在pusers数组分配一个在线用户
    p_users[i] = (User *)malloc(sizeof(User));
    if (p_users[i] == NULL)
    {
        printf("[ERROR]: In UserManager::Login: 登录时分配puser数组失败.\n");
        return false;
    }
    // 建立pid与index的关联
    pid2index[pthread_id] = i;
    p_users[i]->u_uid = 0;
    printf("[INFO]: 线程 %lu 登录成功，用户名为: %s.\n", pthread_id, uname.c_str());
    // 设置 User 结构的初始值
    // 关联根目录
    p_users[i]->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
    p_users[i]->u_cdir->NFrele();
    strcpy(p_users[i]->u_curdir, "/");
    printf("[INFO]：关联根目录成功.\n");
    // 跳转到home目录
    p_users[i]->u_error = NOERROR;
    char home_path[512] = {0};
    std::string tmp = "/home";
    strcpy(home_path, tmp.c_str());
    p_users[i]->u_dirp = home_path;
    p_users[i]->u_arg[0] = (unsigned long long)(home_path);
    FileManager &filemanager = Kernel::Instance().GetFileManager();
    filemanager.ChDir();
    // 创建该用户的家目录
    Kernel::Instance().Sys_Mkdir(uname);
    printf("[INFO]：创建用户 %s 家目录成功.\n", uname.c_str());
    // 跳转到该用户的家目录下对应用户名的目录
    p_users[i]->u_error = NOERROR;
    char home_user_path[512] = {0};
    tmp = "/home/" + uname;
    strcpy(home_user_path, tmp.c_str());
    p_users[i]->u_dirp = home_user_path;
    p_users[i]->u_arg[0] = (unsigned long long)(home_user_path);
    filemanager.ChDir();
    printf("[INFO]：跳转到用户 %s 家目录成功.\n", uname.c_str());
    return true;
}

// 用户登出
bool UserManager::Logout()
{
    // 将该用户行为更新至磁盘
    Kernel::Instance().Quit();

    // 取得当前线程id
    pthread_t pthread_id = pthread_self();
    // 检查该线程是否已登录
    if (pid2index.find(pthread_id) == pid2index.end())
    {
        printf("[ERROR]: 用户线程 %ld 未登录，无需登出\n", pthread_id);
        return false;
    }
    int i = pid2index[pthread_id];
    // 释放puser数组对应记录以及索引表对应记录
    free(p_users[i]);
    pid2index.erase(pthread_id);
    printf("[INFO]: 用户线程 %ld 登出成功.\n", pthread_id);
    return true;
}

// 得到当前线程的User结构
User* UserManager::GetUser()
{
    // 取得当前线程id
    pthread_t pthread_id = pthread_self();
    if (pid2index.find(pthread_id) == pid2index.end())
    {
        printf("[ERROR]: 线程 %ld 的 User 结构未找到.\n", pthread_id);
        exit(1);
    }
    return p_users[pid2index[pthread_id]];
}