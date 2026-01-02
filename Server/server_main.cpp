#include "LoginServer.h"
#include <iostream>
#include <unistd.h>

int main() {
    LoginServer server;
    if (!server.start(6000)) {
        std::cerr << "Server start failed\n";
        return -1;
    }

    std::cout << "Login server running on port 6000\n";
    while (true) sleep(1);
}
