#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    int port = 6001;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;  // 监听公网所有网卡

    if (bind(listenfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen failed");
        return 1;
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientfd = accept(listenfd, (sockaddr*)&clientAddr, &clientLen);
    if (clientfd < 0) {
        perror("accept failed");
        return 1;
    }

    char buffer[1024] = {0};
    int n = read(clientfd, buffer, sizeof(buffer)-1);
    if (n > 0) {
        std::cout << "Received from client: " << buffer << std::endl;
    }

    const char* reply = "Hello from server";
    write(clientfd, reply, strlen(reply));

    close(clientfd);
    close(listenfd);
    return 0;
}

