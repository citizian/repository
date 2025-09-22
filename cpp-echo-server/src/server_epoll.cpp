#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

const int MAX_EVENTS = 10;
const int PORT = 50808;

int make_socket_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        return 1;
    }

    if (make_socket_non_blocking(server_fd) == -1)
    {
        perror("fcntl");
        return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return 1;
    }

    epoll_event event;
    event.data.fd = server_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        perror("epoll_ctl");
        return 1;
    }
    epoll_event events[MAX_EVENTS];
    char buffer[1024];

    std::cout << "Server_Epoll listening on port " << PORT << "..." << std::endl;

    while (true)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                    perror("accept");
                    continue;
                }
                make_socket_non_blocking(client_fd);

                epoll_event client_event;
                client_event.data.fd = client_fd;
                client_event.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);

                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                std::cout << "Client connectd from " << client_ip << ":"
                          << ntohs(client_addr.sin_port) << std::endl;
            }
            else
            {
                int client_fd = events[i].data.fd;
                int count = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (count <= 0)
                {
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                    std::cout << "Client disconnected." << std::endl;
                }
                else
                {
                    buffer[count] = '\0';
                    std::cout << "Received: " << buffer << std::endl;
                    send(client_fd, buffer, count, 0);
                }
            }
        }
    }
    close(server_fd);
    close(epoll_fd);
    return 0;
}