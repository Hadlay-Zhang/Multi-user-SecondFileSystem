/*
 * @Author: ZZL
 * @Date: 2023-05-26 14:41:50
 * @LastEditors: ZZL
 * @LastEditTime: 2023-05-26 18:32:24
 * @Description: 请填写简介
 */
#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <iostream>
#include <string.h>
#include <map>
#include "FileManager.h"
#include "User.h"
#include "File.h"
#include "INode.h"

#define NOERROR 0

class UserManager
{
public:
    static const int USER_N = 100; // 最多支持100个用户同时在线
    UserManager();
    ~UserManager();
    // 用户登录
    bool Login(std::string uname);
    // 用户登出
    bool Logout();
    // 得到当前用户线程的User结构指针
    User* GetUser();
public:
    // 哈希表
    std::map<pthread_t, int> pid2index;
    // User数组，管理所有用户结构
    User *p_users[USER_N];
};

#endif