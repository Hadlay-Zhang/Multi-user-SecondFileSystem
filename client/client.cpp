#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 1024        // 最大缓冲区大小1024字节

void printInitInfo()
{
    printf("====================================\n");
    printf("\033[1;33m多用户二级文件系统 by 2054169 张智淋\033[0m\n");
    printf("====================================\n");
    printf("输入\033[1;33mhelp\033[0m查看所有函数信息与输入格式...\n");
    printf("====================================\n");
};

void Client_Start(int clientfd) {
    printInitInfo();
    char recv_contents[MAX_BUFFER_SIZE] = {0};
    char sendM[MAX_BUFFER_SIZE] = {0};
    bool first_recv = false;
    int ret_numbytes = 0;
    fd_set rfds; // 读事件的文件描述符集合
    bool isfinish = false;

    // 套接字设置为非阻塞模式
    int flags = fcntl(clientfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(clientfd, F_SETFL, flags) < 0) {
        printf("[ERROR]: 设置非阻塞模式错误\n");
        // fprintf(stderr, "Set flags error:%s\n", strerror(errno));
        close(clientfd);
        return;
    }
    // 强制清空stdin缓存
    setbuf(stdin, NULL);
    while(true) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds); // 将标准输入文件描述符加入集合
        FD_SET(clientfd, &rfds); // 将 socket 文件描述符加入集合
        // 使用select函数监听套接字是否可读
        fd_set read_fds; // 定义读取套接字的描述符集合
        FD_ZERO(&read_fds);
        FD_SET(clientfd, &read_fds); // 添加客户端套接字到描述符集合中

        struct timeval tv = {100, 0}; // 超时时间为100秒
        int ret = select(clientfd + 1, &read_fds, NULL, NULL, &tv);
        if (ret == -1) { // select函数调用出错
            printf("[ERROR]: select function error, %s\n", strerror(errno));
            close(clientfd);
            exit(EXIT_FAILURE);
            // return;
        } else if (ret == 0) { // 超时
            // std::cout << "[DEBUG]: 超时." << std::endl;
            continue;
        } else { // 文件描述符集上有事件发生
            if (FD_ISSET(clientfd, &read_fds)) { // 套接字上有数据可读
                int numbytes = recv(clientfd, recv_contents, MAX_BUFFER_SIZE-1, 0);
                // std::cout << "[DEBUG]: numbytes is: " << numbytes << std::endl;
                // std::cout << "[DEBUG]: recv_contents is: " << recv_contents << std::endl;
                if (numbytes == -1) {
                    if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                        printf("[ERROR]: recv error, %s\n", strerror(errno));
                        return;
                    }
                    if (first_recv == false) {
                        continue;
                    }
                    continue;
                } else if (numbytes == 0) {
                    printf("[INFO]: 服务器已关闭连接，客户端退出\n");
                    return;
                } else { // 读取到数据
                    recv_contents[numbytes] = '\0';
                    // std::cout << recv_contents << std::endl;
                    printf("%s", recv_contents);
                    if (first_recv == false) {
                        first_recv = true;
                        // printf("[DEBUG]: first_recv已设置为true.\n");
                    }
                }
            }
        }

        // 用户输入
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (fgets(sendM, MAX_BUFFER_SIZE, stdin)) {
                // std::cin.getline(sendM, MAX_BUFFER_SIZE);
                int send_le = strlen(sendM);
                sendM[send_le - 1] = '\0';
                if(send_le == 1){
                    send_le += 1;
                    sendM[0] = ' ';
                }
                else if(strcmp(sendM, "exit")==0) {
                    isfinish = true;
                }
                // 发送数据
                if ((ret_numbytes = send(clientfd, sendM, send_le - 1, 0)) == -1) {
                    printf("[ERROR]: send error, 错误码: %s\n", strerror(errno));
                    break;
                }
                if (isfinish) {
                    return;
                };
            }
        }
    }
}

int main(int argc, char **argv)
{
    /* 1. 检查用户端ip地址和端口号输入 */
    if (argc < 3) {
        printf("请按照 [./client] [ip地址] [端口号] 的格式输入指令，例: ./client localhost 1234\n");
        return -1;
    }
    char* ip_addr = argv[1];                //ip地址
    unsigned int port = atoi(argv[2]); //端口号

    /* 2. 创建client端socket */
    int clientfd = 0;
    // 定义客户端地址结构体
    struct sockaddr_in addr;
    // socket函数创建socket并检查
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0){
        fprintf(stderr, "[ERROR]: 创建客户端socket失败: %s.\n", strerror(errno));
        return -1;
    }

    /* 3. 设置socket属性 */
    // 清空服务端地址结构体
    bzero(&addr, sizeof(addr));
    // 设置使用的协议族为IPv4
    addr.sin_family = AF_INET;
    // 设置端口
    addr.sin_port = htons(port);
    // 将IP地址字符串转换为结构体的二进制形式，并存储在addr的sin_addr成员中
    inet_pton(AF_INET, ip_addr, &addr.sin_addr);

    /* 4. 设置sockct为非阻塞 */
    // 获取fd状态标志
    int flags = fcntl(clientfd, F_GETFL, 0);
    // 设置为非阻塞
    flags |= O_NONBLOCK;
    // 设置状态标志
    if (fcntl(clientfd, F_SETFL, flags) < 0){
        fprintf(stderr, "[ERROR]: 设置非阻塞状态失败: %s\n", strerror(errno));
        close(clientfd);
        return -1;
    }

    /* 5. 建立连接 */
    int connect_cnt = 1;    // 重连计数器
    while(1){
        int returnflag = connect(clientfd, (struct sockaddr *)&addr, sizeof(addr));
        // 连接成功
        if(returnflag == 0){
            printf("[INFO]: 与服务端连接成功.\n");
            break;
        }
        if(connect_cnt > 10){
            printf("[ERROR]: 与服务端连接失败.\n");
            return 0;
        }
        printf("[INFO]: 第 %d 次连接服务端失败，正在尝试重连...\n", connect_cnt++);
    }
    /* 6. 客户端开始运行文件系统服务 */
    Client_Start(clientfd);

    /* 7. 客户端结束服务 */
    close(clientfd);
    printf("[INFO]: 客户端关闭连接，退出成功.\n");
    return 0;
}
