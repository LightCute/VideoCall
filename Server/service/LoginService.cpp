//LoginService
#include "LoginService.h"

LoginResult LoginService::handleLogin(const event::LoginRequest& req) {
    LoginResult r;

    if (req.username == "admin" && req.password == "123") {
        r.success = true;
        r.username = "admin";
        r.privilege = 10;
    } 
    else if (req.username == "user_a" && req.password == "123") {
        r.success = true;
        r.username = "user_a";
        r.privilege = 10;
    } 
    else if (req.username == "user_b" && req.password == "123") {
        r.success = true;
        r.username = "user_b";
        r.privilege = 10;
    }     
    else if (req.username == "user_c" && req.password == "123") {
        r.success = true;
        r.username = "user_c";
        r.privilege = 10;
    }     


    else {
        r.success = false;
        r.reason = "invalid username or password";
    }

    return r;
}
