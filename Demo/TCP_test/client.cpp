#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    const char* serverIp = "120.79.210.6"; // 替换成真实公网IP
    int port = 6001;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton failed");
        return 1;
    }

    if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect failed");
        return 1;
    }

    const char* msg = "Hello from client";
    write(sockfd, msg, strlen(msg));

    char buffer[1024] = {0};
    int n = read(sockfd, buffer, sizeof(buffer)-1);
    if (n > 0) {
        std::cout << "Received from server: " << buffer << std::endl;
    }

    close(sockfd);
    return 0;
}

