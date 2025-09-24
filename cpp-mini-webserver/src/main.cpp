// src/main.cpp
#include "Config.h"
#include "Logger.h"
#include "Server.h"
#include <iostream>

int main() {
    // 1. 读取配置
    Config config;
    if (!config.load("config/server.conf")) {
        std::cerr << "Failed to load config file!" << std::endl;
        return 1;
    }

    // 2. 初始化 Logger
    Logger logger(config.getString("log_file", "./logs/server.log"));
    logger.info("Starting Mini WebServer...");

    // 3. 输出配置信息
    int port = config.getInt("port", 8080);
    int threadNum = config.getInt("thread_num", 4);
    std::string webRoot = config.getString("web_root", "./www");

    logger.info("Port: " + std::to_string(port));
    logger.info("Thread pool size: " + std::to_string(threadNum));
    logger.info("Web root: " + webRoot);

    // 4. 创建并初始化 Server
    Server server(config);
    if (!server.init()) {
        logger.error("Server initialization failed!");
        return 1;
    }

    logger.info("Server initialized successfully, waiting for connections...");

    // 5. 启动 Server 主循环（epoll + 线程池）
    server.run();

    return 0;
}
