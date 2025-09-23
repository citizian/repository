#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

const int PORT = 50808;
const int MAX_EVENTS = 64;
const int BUF_SIZE = 4096;
int make_non_blocking (int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void send_all(const std::vector<int>& fds, const std::string& msg){
    for(int fd : fds){
        send(fd,msg.c_str(), msg.size(), 0);
    }
}

std::string build_online_line(const std::unordered_map<std::string,int>& id2fd){
    std::string s;
    bool first = true;
    for(const auto& kv : id2fd){
        if(!first) s.push_back(',');
        s += kv.first;
        first = false;
    }
    return "ONLINE: "+ s + "\n";
}
int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) { perror("socket");return 1;}

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd,(sockaddr*)&addr,sizeof(addr)) == -1){perror("bind");return 1;}
    if(listen(server_fd, SOMAXCONN) == -1){perror("listen");return 1;}
    if(make_non_blocking(server_fd) == -1){perror("fcntl"); return 1;}
    
    int epfd = epoll_create1(0);
    if(epfd == -1){perror("epoll_create1");return 1;}

    epoll_event ev{};
    ev.data.fd = server_fd;
    ev.events = EPOLLIN;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,server_fd, &ev) == -1){perror("epoll_ctl add server"); return 1;}

    std::unordered_map<int, std::string> fd2id;
    std::unordered_map<std::string, int> id2fd;
    std::unordered_map<int, std::string> inbuf;
    int next_id = 1;

    std::cout<< "Private chat epoll server listening on port " << PORT << "...\n";

    epoll_event events[MAX_EVENTS];
    while(true){
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if(n == -1){perror("epoll_wait");break;}
        for(int i =0; i<n;++i){
            int fd = events[i].data.fd;
            if(fd == server_fd){
                while(true){
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr*)&client_addr,&client_len);
                    if(client_fd == -1){
                        break;
                    }
                    make_non_blocking(client_fd);
                    epoll_event client_ev{};
                    client_ev.data.fd = client_fd;
                    client_ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);

                    std::string id = "user" + std::to_string(next_id++);
                    fd2id[client_fd] = id;
                    id2fd[id] = client_fd;
                    inbuf[client_fd] = "";

                    std::string assign = "ASSING: " + id + "\n";
                    send(client_fd,assign.c_str(), assign.size(), 0);

                    std::string online = build_online_line(id2fd);
                    std::vector<int> all_fds;
                    for(const auto& kv: fd2id) all_fds.push_back(kv.first);
                    send_all(all_fds, online);

                    char ipbuf[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf,sizeof(ipbuf));
                    std::cout << "New Connection: fd=" << client_fd << " id ="<< id
                    << " from " << ipbuf << ":" << ntohs(client_addr.sin_port) << "\n";
                }
            }else{
                if(events[i].events & (EPOLLRDHUP | EPOLLHUP)){
                    std::string id = fd2id.count(fd) ? fd2id[fd] : "";
                    if(!id.empty()){
                        std::cout << "Client disconnect: " << id <<" fd= " << fd <<"\n";
                        fd2id.erase(fd);
                        id2fd.erase(id);
                    }
                    epoll_ctl(epfd,EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    inbuf.erase(fd);
                    std::string online = build_online_line(id2fd);
                    std::vector<int> all_fds;
                    for(const auto& kv : fd2id) all_fds.push_back(kv.first);
                    send_all(all_fds,online);
                    continue;
                }
            }
            char buf[BUF_SIZE];
            while(true){
                ssize_t count = recv(fd, buf, sizeof(buf) -1, 0);
                if(count == -1){
                    break;
                }else if(count == 0){
                    std::string id =fd2id.count(fd) ? fd2id[fd] : "";
                    if(!id.empty()){
                        std::cout << "Client disconnect (recv 0): " << id << " fd=" << fd <<"\n";
                        fd2id.erase(fd);
                        id2fd.erase(id);
                    }
                    epoll_ctl(epfd, EPOLL_CTL_DEL,fd,nullptr);
                    close(fd);
                    inbuf.erase(fd);
                    std::string online = build_online_line(id2fd);
                    std::vector<int> all_fds;
                    for(const auto& kv : fd2id) all_fds.push_back(kv.first);
                    send_all(all_fds, online);
                    break;
                }else{
                    buf[count] = '\0';
                    inbuf[fd] += std::string (buf,count);
                    std::string &s = inbuf[fd];
                    size_t pos;
                    while((pos = s.find('\n')) != std::string::npos){
                        std::string line = s.substr(0,pos);
                        s.erase(0,pos+1);
                        if(line.size() == 0) continue;
                        if(line.rfind("TO:",0) == 0){
                            std::string payload = line.substr(3);
                            size_t bar = payload.find('|');
                            if(bar == std::string::npos) {
                                std::string err = "ERROR: Bad message format\n";
                                send(fd,err.c_str(),err.size(),0);
                                continue;
                            }
                            std::string target = payload.substr(0, bar);
                            std::string content = payload.substr(bar +1);
                            std::string from = fd2id.count(fd) ? fd2id[fd] : "unknown";
                            if(id2fd.count(target)){
                                int target_fd = id2fd[target];
                                std::string forward = "CHAT:" + from + "|" + content + "\n";
                                send(target_fd, forward.c_str(), forward.size(), 0);
                            }else{
                                std::string err = "ERROR:User not online.\n";
                                send(fd, err.c_str(), err.size(), 0);
                            }
                        }else {
                            std::string err = "ERROR: Unkown command.\n";
                            send(fd, err.c_str(), err.size(), 0);
                        }
                        
                    }
                }
            }
        }
    }
    close(server_fd);
    close(epfd);
    return 0;
}