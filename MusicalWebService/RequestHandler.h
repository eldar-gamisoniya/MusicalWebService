#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <string>
#include <fastcgi++/request.hpp>
#include <stdlib.h>

#include "rapidjson/prettywriter.h"
#include "UserModel.h"
#include "AudioModel.h"
#include "PlaylistModel.h"
#include "DatabaseHandler.h"

class RequestHandler: public Fastcgipp::Request<char>
{
private:
    DatabaseHandler dbHandler;

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

    bool addPlaylist();
    bool getPlaylists();
    bool getPlaylist();
    bool deletePlaylist();
    bool addAudioToPlaylist();
    bool deleteAudioFromPlaylist();

    std::string getId(int offset);
    bool checkUser();

    bool writeMusic(const std::string& file);
    void writeUser(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, UserModel& user);
    void writeAudio(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, AudioModel& audio);
    void writePlaylist(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, PlaylistModel& playlist, bool audios);

public:
    bool response();
};

#endif
