#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include "UserModel.h"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>

class DatabaseHandler
{
private:
    static std::string newUuid();
    static std::string elementToString(bsoncxx::document::element el);
    static UserModel modifyToken(std::string login, std::string password, std::string newToken);

public:
    static UserModel createUser(std::string login, std::string password, std::string aboutYourSelf);
    static UserModel getUserByLogin(std::string login);
    static UserModel connect(std::string login, std::string password);
    static UserModel disconnect(std::string login, std::string password);
};

#endif
