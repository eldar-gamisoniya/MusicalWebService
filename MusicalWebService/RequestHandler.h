#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <string>
#include <fastcgi++/request.hpp>
#include <stdlib.h>

#include "rapidjson/prettywriter.h"
#include "UserModel.h"
#include "AudioModel.h"

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

    bool addAudio();
    bool getAudio();
    bool getAudios();
    bool getStream();

    std::string getId(int offset);
    bool checkUser();

    bool writeMusic(const std::string& file);
    void writeUser(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, UserModel& user);
    void writeAudio(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, AudioModel& audio);

public:
    bool response();
};

#endif
