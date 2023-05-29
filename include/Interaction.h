/*
 * @Author: ZZL
 * @Date: 2023-05-26 14:41:50
 * @LastEditors: ZZL
 * @LastEditTime: 2023-05-26 18:34:28
 * @Description: 请填写简介
 */
#ifndef INTERACTION_H
#define INTERACTION_H

#include <unordered_map>
#include <string>

// 用于解析用户指令的switch结构
typedef enum {
    API_NULL,//空
    API_SPACE,//空格
    API_HELP,
    API_CD,
    API_LS,
    API_TOUCH,
    API_MKDIR,
    API_RM,
    API_OPEN,
    API_CLOSE,
    API_READ,
    API_WRITE,
    API_SEEK,
    API_CAT,
    API_CPIN,
    API_CPOUT,
    API_EXIT
}ApiType;

class Interaction
{
private:
public:
    bool isNumber(const std::string &str);
    std::string printInitInfo();
    void printHelp(std::stringstream &input_s, int socketfd, std::string username);
    void Api_cd(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_ls(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_mkdir(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_touch(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_rm(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_open(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_close(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_write(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_read(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_seek(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_cat(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_cpin(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    void Api_cpout(std::stringstream &input_s, std::stringstream &return_s, int socketfd, std::string username);
    int send2client(int fd, std::string username, std::string contents);
    void* MainLoop(void* arg);
    static void* ThreadStart(void* arg);
    void Entrance();
};

#endif
