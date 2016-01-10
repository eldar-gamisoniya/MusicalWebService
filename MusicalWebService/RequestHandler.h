#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <string>
#include <fastcgi++/request.hpp>
#include <stdlib.h>

#include "rapidjson/prettywriter.h"
#include "UserModel.h"

class RequestHandler: public Fastcgipp::Request<char>
{
private:
    void setReturnCode(const std::string& code);
    void setHttpHeaders(const std::string& code);

    bool addUser();
    bool connectUser();
    bool disconnectUser();
    bool getUser();
    bool getUsers();
    bool changeUserInfo();
    bool changePassword();

    std::string getId(int offset);

    bool writeMusic(const char* file, int size);
    void writeUser(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, UserModel& user);

public:
    bool response();
};

#endif
