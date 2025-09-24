// src/Server.cpp
#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Utils.h"

Server::Server(const Config& config)
    : threadPool(config.getInt("thread_num", 4)),
      logger(config.getString("log_file", "./logs/server.log")),
      webRoot(config.getString("web_root", "./www")),
      port(config.getInt("port", 8080)) {
    listenFd = -1;
    epollFd = -1;
}

Server::~Server() {
    if (listenFd != -1) close(listenFd);
    if (epollFd != -1) close(epollFd);
}

bool Server::init() {
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        logger.error("Failed to create socket");
        return false;
    }

    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setNonBlocking(listenFd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        logger.error("Bind failed");
        return false;
    }

    if (listen(listenFd, 128) < 0) {
        logger.error("Listen failed");
        return false;
    }

    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        logger.error("epoll_create1 failed");
        return false;
    }

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listenFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev) < 0) {
        logger.error("epoll_ctl ADD listenFd failed");
        return false;
    }

    logger.info("Server initialized on port " + std::to_string(port));
    return true;
}

void Server::run() {
    const int MAX_EVENTS = 1024;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            logger.error("epoll_wait error");
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            if (fd == listenFd) {
                handleAccept();
            } else if (events[i].events & EPOLLIN) {
                handleRead(fd);
            } else if (events[i].events & EPOLLOUT) {
                handleWrite(fd);
            } else {
                closeConnection(fd);
            }
        }
    }
}

void Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::handleAccept() {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int connFd;

    while ((connFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientLen)) > 0) {
        setNonBlocking(connFd);

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = connFd;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, connFd, &ev);

        clients[connFd] = ClientConnection{connFd, "", ""};
        logger.info("Accepted connection fd=" + std::to_string(connFd));
    }
}



void Server::handleRead(int fd) {
    auto it = clients.find(fd);
    if (it == clients.end()) return;

    char buffer[4096];
    while (true) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n > 0) {
            it->second.readBuffer.append(buffer, n);
        } else if (n == 0) {
            closeConnection(fd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            closeConnection(fd);
            return;
        }
    }

    threadPool.enqueue([this, fd]() {
        auto it2 = clients.find(fd);
        if (it2 == clients.end()) return;

        HttpRequest request;
        if (!request.parse(it2->second.readBuffer)) {
            closeConnection(fd);
            return;
        }

        HttpResponse response;

        std::string filePath = utils::joinPath(webRoot, request.path == "/" ? "index.html" : request.path.substr(1));
        std::string content = utils::readFile(filePath);

        if (content.empty()) {
            response.setStatus(404, "Not Found");
            response.setBody("<h1>404 Not Found</h1>");
        } else {
            response.setStatus(200, "OK");
            response.setHeader("Content-Type", "text/html");
            response.setBody(content);
        }

        it2->second.writeBuffer = response.toString();

        epoll_event ev{};
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
    });
}

/*
void Server::handleRead(int fd) {
    auto it = clients.find(fd);
    if (it == clients.end()) return;

    char buffer[4096];
    while (true) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n > 0) {
            it->second.readBuffer.append(buffer, n);
        } else if (n == 0) {
            closeConnection(fd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            closeConnection(fd);
            return;
        }
    }

    // 提交任务给线程池处理
    threadPool.enqueue([this, fd]() {
        auto it2 = clients.find(fd);
        if (it2 == clients.end()) return;

        // 简单 echo 响应（后续替换为 HTTP Response）
        it2->second.writeBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";

        // 修改 epoll 事件为可写
        epoll_event ev{};
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
    });
}
*/

void Server::handleWrite(int fd) {
    auto it = clients.find(fd);
    if (it == clients.end()) return;

    std::string &data = it->second.writeBuffer;
    while (!data.empty()) {
        ssize_t n = send(fd, data.c_str(), data.size(), 0);
        if (n > 0) {
            data.erase(0, n);
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            closeConnection(fd);
            return;
        }
    }

    if (data.empty()) {
        // 切换回可读事件
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
    }
}

void Server::closeConnection(int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    clients.erase(fd);
    logger.info("Closed connection fd=" + std::to_string(fd));
}
