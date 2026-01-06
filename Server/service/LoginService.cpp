//LoginService
#include "LoginService.h"

LoginResult LoginService::handleLogin(const LoginRequest& req) {
    LoginResult r;

    if (req.username == "admin" && req.password == "123") {
        r.success = true;
        r.username = "admin";
        r.privilege = 10;
    } else {
        r.success = false;
        r.reason = "invalid username or password";
    }

    return r;
}
