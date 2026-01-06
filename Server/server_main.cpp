//server_main.cpp
#include "./core/LoginServer.h"
#include <iostream>
#include <unistd.h>

int main() {
    LoginServer server;
    if (!server.start(6001)) {
        std::cerr << "Server start failed\n";
        return -1;
    }

    std::cout << "Login server running on port 6001\n";
    while (true) sleep(1);
}
