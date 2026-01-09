//User.h
#pragma once
#include <string>


namespace domain {

struct User {
    std::string username;
    int privilege = 0;
};

} // namespace domain
