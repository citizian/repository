#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]){
    if(argc > 2){
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int port = 50808;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1){
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0){
        perror("inet_pton");
        return 1;
    };

    if(connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        return 1;
    }
    std::cout << "Connected to " << server_ip << ":"<<port<<std::endl;
    std::cout << "Type message and press Enter (Ctrl+C to quit)" << std::endl;

    char buffer[1024];
while(true){
    std::string input;
    std::cout << ">";
    std:getline(std::cin, input);
    if(input.empty()) continue;

    send(sock, input.c_str(), input.size(), 0);

    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if(n > 0){
        buffer[n] = '\n';
        std::cout << "Received from server: " << buffer << std::endl;
    }else{
        std::cout << "Server closed connection." << std::endl;
        break;
    }
}
    close(sock);
    return 0;
}