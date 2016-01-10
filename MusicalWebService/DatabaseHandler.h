#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include "UserModel.h"
#include "StaticConnection.h"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <vector>


class DatabaseHandler
{
private:
    static std::string newUuid();
    static std::string elementToString(const bsoncxx::document::element& el);
    static long elementToLong(const bsoncxx::document::element& el);
    static long currentTimestamp();
    static StaticConnection connection;

public:
    //User operations
    static std::vector<UserModel> getUsers(const std::string& loginRegex, const std::string& id,
                                           const long timestampFrom, const long count);
    static UserModel createUser(const std::string& login,const std::string& password,
                                const std::string& aboutYourSelf);
    static UserModel modifyAboutYourSelf(const std::string& login, const std::string& token,
                                         const std::string& aboutYourSelf);
    static UserModel modifyPassword(const std::string& login, const std::string& token,
                                    const std::string& oldPassword, const std::string& newPassword);
    static UserModel getUserByLogin(const std::string& login);
    static UserModel createToken(const std::string& login, const std::string& password);
    static UserModel deleteToken(const std::string& login, const std::string& token);

    //Music operations

};

#endif
