#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(50808);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd,(sockaddr*)&addr, sizeof(addr)) == -1){
        perror("bind");
        return 1;
    }

    if(listen(server_fd,5) == -1){
        perror("listen");
        return 1;
    }

    std::cout << "Server listening on port 50808..." << std::endl;

while(true){
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if(client_fd == -1){
        perror("accept");
        continue;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    std::cout << "Client connectd from " << client_ip<<":"
                << ntohs(client_addr.sin_port) << std::endl;

    char buffer[1024];
    while(true){
        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if(n > 0){
            buffer[n] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            send(client_fd, buffer, n, 0);
        }else{
            std::cout << "Client disconnected." << std::endl;
            break;
        }
    }
    close(client_fd);
}
    close(server_fd);

    return 0;

}