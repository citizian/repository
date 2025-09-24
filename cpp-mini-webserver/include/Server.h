// include/Server.h
#pragma once
#include "ThreadPool.h"
#include "Logger.h"
#include "Config.h"
#include "Utils.h"

#include <string>
#include <unordered_map>
#include <sys/epoll.h>

struct ClientConnection {
    int fd;                       // 客户端 socket
    std::string readBuffer;       // 读缓冲区
    std::string writeBuffer;      // 写缓冲区
};

class Server {
public:
    Server(const Config& config);
    ~Server();

    bool init();          // 初始化服务器（socket/epoll/线程池）
    void run();           // 启动 epoll 主循环

private:
    int listenFd;                     // 监听 socket
    int epollFd;                       // epoll fd
    ThreadPool threadPool;             // 线程池
    std::unordered_map<int, ClientConnection> clients; // 活跃客户端
    Logger logger;
    std::string webRoot;
    int port;

    void setNonBlocking(int fd);
    void handleAccept();
    void handleRead(int fd);
    void handleWrite(int fd);
    void closeConnection(int fd);
};
