#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include "UserModel.h"
#include "AudioModel.h"
#include "StaticConnection.h"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <vector>
#include <memory>


class DatabaseHandler
{
private:
    static std::string newUuid();
    static std::string elementToString(const bsoncxx::document::element& el);
    static std::string elementToStringId(const bsoncxx::document::element& el);
    static long elementToLong(const bsoncxx::document::element& el);
    static long elementToDate(const bsoncxx::document::element& el);
    static long currentTimestamp();
    static StaticConnection connection;

public:
    //User operations
    static std::vector<UserModel> getUsers(const std::string& loginRegex, const int skipCount, const int count);
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
    static AudioModel createAudio(const std::string& owner, const std::string& name, const std::string& description,
                                  const unsigned char* data, const int size);
    static std::shared_ptr<AudioModel> getAudio(const std::string& id, bool getData);
    static std::vector<AudioModel> getAudios(const std::string& nameRegex, const std::string& ownerRegex,
                                             const int skipCount, const int count);
};

#endif
