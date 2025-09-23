#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>

#include<iostream>
#include<string>
#include<thread>
#include<mutex>
#include<atomic>

std::mutex print_mtx;
std::atomic<bool> running{true};

void safe_print(const std::string& s){
    std::lock_guard<std::mutex> lk(print_mtx);
    std::cout << s << "\n";
}

void recv_thread_func(int sockfd){
    char buf[4096];
    std::string inbuf;
    while(running){
        ssize_t n = recv(sockfd, buf, sizeof(buf)-1, 0);
        if(n > 0){
            buf[n] = '\0';
            inbuf += std::string(buf, n);
            size_t pos;
            while((pos = inbuf.find('\n')) != std::string::npos){
                std::string line = inbuf.substr(0,pos);
                inbuf.erase(0, pos +1);
                if(line.rfind("ASSIGN:", 0) ==0){
                    std::string id = line.substr(7);
                    safe_print("[System] Your ID = " + id);
                }else if(line.rfind("ONLINE:",0) == 0){
                    std::string list = line.substr(7);
                    safe_print("[System] Online: " + list);
                }else if(line.rfind("CHAT:",0) == 0){
                    std::string payload = line.substr(5);
                    size_t bar = payload.find('|');
                    if(bar != std::string::npos){
                        std::string from = payload.substr(0,bar);
                        std::string content = payload.substr(bar + 1);
                        safe_print("[" + from + "] " + content);
                    }else{
                        safe_print("[Chat] "+payload);
                    }
                }else if(line.rfind("ERROR:",0) == 0){
                    std::string err = line.substr(6);
                    safe_print("[Error] " + err);
                }else{
                    safe_print("[Recv] " + line);
                }
            }
        }else if(n == 0){
            safe_print("[System] Server closed connection.");
            running = false;
            break;
        }else {
            safe_print("[System] recv error or interrupted.");
            running = false;
            break;
        }
    }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << "<server_ip> [port]\n";
        return 1;
    }
    const char * server_ip = argv[1];
    int port = 50808;
    if(argc >= 3){
        port = atoi(argv[2]);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){perror("socket"); return 1;}

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0 ){
        perror("inet_pton");
        return 1;
    }
    if(connect(sock, (sockaddr*) &server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        return 1;
    }
    safe_print("[System] Connected to server " + std::string(server_ip) + ":" + std::to_string(port));
    safe_print("Usage:");
    safe_print("  @target message    -- send private message to target (e.g. @user2 hi)");
    safe_print("  /quit              -- exit");
    std::thread recv_thread(recv_thread_func,sock);

    while(running){
        std::string line;
        {
            std::lock_guard<std::mutex> lk(print_mtx);
            std::cout << ">";
            std::cout.flush();
        }
        if(!std::getline(std::cin,line)){
            running = false;
            break;
        }
        if(line.size() == 0) continue;
        if(line == "/quit"){
            running = false;
            break;
        }

        if(line[0] == '@'){
            size_t space = line.find(' ');
            if(space == std::string::npos){
                safe_print("[System] Bad Input. Use: @target message");
                continue;
            }
            std::string target = line.substr(1, space-1);
            std::string msg = line.substr(space + 1);
            std::string out = "TO:"+target+"|"+msg+"\n";
            ssize_t sent = send(sock, out.c_str(),out.size(),0);
            if(sent <= 0 ){
                safe_print("[System] send failed.");
                running = false;
                break;
            }
        }else{
            safe_print("[System] Unknown input. Use @target message or /quit");

        }
    }
    close(sock);
    if(recv_thread.joinable()) recv_thread.join();
    safe_print("[System] client exiting.");
    return 0;
}