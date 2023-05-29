/*
 * @Author: ZZL
 * @Date: 2023-05-26 14:41:50
 * @LastEditors: ZZL
 * @LastEditTime: 2023-05-26 17:35:27
 * @Description: 请填写简介
 */
#ifndef DISKFILEMANAGER_H
#define DISKFILEMANAGER_H
#include "FileSystem.h"
#include "BufferManager.h"

class DiskFileManager
{
private:
    const char* DiskFilePath = "MyDisk.img";//磁盘路径
    size_t TotalSize;//磁盘文件大小
    int Diskfd;//文件描述符
    BufferManager *m_BufferManager;
public:
    DiskFileManager();
    ~DiskFileManager();
    void Init_Superblock(SuperBlock* &superblock);  //初始化超级块
    void Init_INode(DiskInode* &dinode);            //初始化Inode区
    void Init_DataBlock(char* data);                //初始化DataBlock区
    void SetImg();                                  //格式化磁盘
    void GetImg();                                  //读取磁盘
    void Initialize();                              //磁盘初始化总函数
    void Quit();                                    //退出文件系统磁盘处理
};

#endif