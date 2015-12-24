#ifndef USERMODEL_H
#define USERMODEL_H

#include<string>

class UserModel
{
public:
    std::string login;
    std::string password;
    std::string aboutYourSelf;
    std::string token;
    std::string id;
    bool isValid;
};

#endif
