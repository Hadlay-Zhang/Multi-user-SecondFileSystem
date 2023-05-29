#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/DiskFileManager.h"
#include "../include/Kernel.h"

DiskFileManager::DiskFileManager()
{
    this->TotalSize = sizeof(SuperBlock) + sizeof(DiskInode) * FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR + FileSystem::DATA_ZONE_SIZE * 512;
}

DiskFileManager::~DiskFileManager()
{
    // nothing to do here
}

/* 初始化SuperBlock */
void DiskFileManager::Init_Superblock(SuperBlock* &superblock)
{
    /* 初始化外存Inode区占用的盘块数和盘块总数 */
    superblock->s_isize = FileSystem::INODE_ZONE_SIZE;
    superblock->s_fsize = FileSystem::DATA_ZONE_END_SECTOR + 1;
    /* 初始化直接管理的外存Inode */
    superblock->s_ninode = 100;
    for (int i = 0; i < superblock->s_ninode; i++)
        superblock->s_inode[i] = i ;//注：这里只是diskinode的编号，真正取用的时候要进行盘块的转换
    /* 初始化其它属性 */
    //superblock->s_flock = 0;
    //superblock->s_ilock = 0;
    superblock->s_fmod = 0;
    superblock->s_ronly = 0;
    superblock->s_time = 0;
    memset(superblock->padding, 0, sizeof(superblock->padding));
    /* 计算SuperBlock直接管理多少个块 */
    int last_group_size = 0;
    if (FileSystem::DATA_ZONE_SIZE < 99) {
        last_group_size = FileSystem::DATA_ZONE_SIZE;
    }
    else {
        last_group_size = (FileSystem::DATA_ZONE_SIZE - 99) % 100;
    }
    superblock->s_nfree = last_group_size;
    /* 计算最后一组的第一个块的编号 */
    int last_group_start = FileSystem::DATA_ZONE_START_SECTOR;
    while(last_group_start + 99 < FileSystem::DATA_ZONE_END_SECTOR) {
        last_group_start += 100;
    }
    last_group_start -= 1;
    /* 填入SuperBlock直接管理的块编号 */
    for (int i = 0;i < superblock->s_nfree;i++) {
        superblock->s_free[i] = last_group_start + i;
    }
}

/* 初始化Inode区 */
void DiskFileManager::Init_INode(DiskInode* &dinode)
{
    dinode = new DiskInode[FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR];
    //初始化rootDiskInode
    dinode[0].d_mode = Inode::IFDIR | Inode::IEXEC;
}

/* 初始化Datablock区 */
void DiskFileManager::Init_DataBlock(char* data) {
    /* 索引表结构体 */
    typedef struct {
        int nfree;      //本组空闲块个数
        int free[100];  //本组空闲块索引表
    }index_table;
    /* 实例化索引表结构 */
    index_table idt;
    /* 未加入索引的盘块的数量 */
    int num_db_left = FileSystem::DATA_ZONE_SIZE;
    /* 初始化组长盘块 */
    for(int i = 0; num_db_left > 0; i++)
    {
        if (num_db_left < 100)
            break;
        idt.nfree = 100;
        // idt.nfree = num_db_left >= 100 ? 100 : num_db_left;
        num_db_left -= idt.nfree;
        /* 填入空闲块索引表 */
        for (int j = 0; j < idt.nfree; j++)
        {
            /* 第一组只有99个块 */
            if (i == 0 && j == 0)
                idt.free[j] = 0;
            else
                idt.free[j] = 100 * i + j + FileSystem::DATA_ZONE_START_SECTOR - 1;
        }
        memcpy(&data[i * 100 * 512 + 99 * 512], (void *)&idt, sizeof(index_table));
    }
}

/* 格式化磁盘文件 */
void DiskFileManager::SetImg()
{
    // 初始化超级块
    SuperBlock *superblock = new SuperBlock;
    this->Init_Superblock(superblock);
    printf("[INFO]: SuperBlock区初始化完成.\n");

    //初始化INode区
    DiskInode* dinode = NULL;
    this->Init_INode(dinode);
    printf("[INFO]: INode区初始化完成.\n");

    //初始化DataBlock区
    char *datablock = new char[FileSystem::DATA_ZONE_SIZE * 512];
    memset(datablock, 0, FileSystem::DATA_ZONE_SIZE * 512);
    this->Init_DataBlock(datablock);
    printf("[INFO]: DataBlock区初始化完成.\n");

    // ftruncate(this->Diskfd, this->TotalSize);
    if (ftruncate(this->Diskfd, this->TotalSize) == -1) {
        perror("[ERROR]: Ftruncate Error!\n");
        return;
    }
    // mmap建立映射
    char *p = (char *)mmap(NULL, this->TotalSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->Diskfd, 0);

    size_t size_sb = sizeof(SuperBlock);
    size_t size_inode = sizeof(DiskInode) * FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR;
    size_t size_db = FileSystem::DATA_ZONE_SIZE * 512;
    memcpy(p, superblock, size_sb);
    memcpy(p + size_sb, dinode, size_inode);
    memcpy(p + size_sb + size_inode, datablock, size_db);
    printf("[INFO]: SuperBlock、INode区、DataBlock区写入完成.\n");
    printf("[INFO]: SuperBlock区大小为: %ld 字节.\n", static_cast<long int>(size_sb));
    printf("[INFO]: INode区大小为: %ld 字节.\n", static_cast<long int>(size_inode));
    printf("[INFO]: DataBlock区大小为: %ld 字节.\n", static_cast<long int>(size_db));
    printf("[INFO]: 磁盘文件总大小为: %ld 字节.\n", static_cast<long int>(this->TotalSize));
    // 释放申请的动态内存
    delete superblock;
    delete[] dinode;
    delete[] datablock;

    /* 设置BufferManager映射指针 */
    this->m_BufferManager->SetMapAddr(p);

    return;
}

/* 读取磁盘文件 */
void DiskFileManager::GetImg()
{
    // 查看读入磁盘大小
    struct stat st;
    fstat(this->Diskfd, &st);
    printf("[INFO]: 读入的磁盘文件 %s 的大小为: %ld\n", this->DiskFilePath, st.st_size);
    // 构建mmap映射
    char *addr = (char *)mmap(NULL, this->TotalSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->Diskfd, 0);
    /* 让BufferManager拥有映射指针 */
    this->m_BufferManager->SetMapAddr(addr);
}

/* 磁盘初始化总函数 */
void DiskFileManager::Initialize()
{
    printf("[INFO]: 开始初始化磁盘文件...\n");
    // 获得BufferManger
    this->m_BufferManager = &Kernel::Instance().GetBufferManager();
    int fd = open(this->DiskFilePath, O_RDWR);
    // 若不存在磁盘文件，则创建新的磁盘文件
    if (fd == -1)
    {
        fd = open(this->DiskFilePath, O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            printf("[ERROR]: 创建 %s 磁盘文件失败\n", this->DiskFilePath);
            exit(-1);
        }
        this->Diskfd = fd;
        // 对磁盘进行初始化
        printf("[INFO]: 未找到磁盘文件，开始格式化磁盘...\n");
        this->SetImg();
        printf("[INFO]: 磁盘格式化完成.\n");
    }
    // 已存在磁盘文件，则直接读入即可
    else
    {
        printf("[INFO]: 找到磁盘文件，开始读入磁盘...\n");
        this->Diskfd = fd;
        this->GetImg();
        printf("[INFO]: 磁盘读入完成.\n");
    }
}

/* 退出时磁盘处理函数，主要为解除mmap映射 */
void DiskFileManager::Quit()
{
    // 获取映射的起始地址
    char *p = this->m_BufferManager->GetMapAddr();
    // 使用msync写回
    msync((void *)p, this->TotalSize, MS_SYNC);
    // 解除映射
    munmap(p, this->TotalSize);
}