#include <regex>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/stringbuffer.h"

#include "RequestHandler.h"
#include "DatabaseHandler.h"
#include "ReturnCodes.h"

bool RequestHandler::response()
{
    static std::regex userRegex("/api/v1/users/[0-9a-zA-Z]+");
    static std::regex tokensRegex("/api/v1/users/[0-9a-zA-Z]+/token");
    static std::regex tokenRegex("/api/v1/users/[0-9a-zA-Z]+/token/[0-9a-z\\-]+");
    static std::regex passwordRegex("/api/v1/users/[0-9a-zA-Z]+/password");
    static std::regex audioRegex("/api/v1/audios/[0-9a-z]+");
    static std::regex streamRegex("/api/v1/audios/[0-9a-z]+/stream");
    static std::regex playlistRegex("/api/v1/playlists/[0-9a-z]+");
    static std::regex playlistAudiosRegex("/api/v1/playlists/[0-9a-z]+/audios");
    static std::regex playlistAudioRegex("/api/v1/playlists/[0-9a-z]+/audios/[0-9a-z]+");


    static char usersUri[] = "/api/v1/users";
    static char audiosUri[] = "/api/v1/audios";
    static char playlistsUri[] = "/api/v1/playlists";

    //static std::string test;

    switch (environment().requestMethod)
    {
    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_GET:
        if (environment().scriptName.compare(usersUri) == 0)
            return getUsers();

        if (environment().scriptName.compare(audiosUri) == 0)
            return getAudios();

        if (environment().scriptName.compare(playlistsUri) == 0)
            return getPlaylists();

        if (std::regex_match(environment().scriptName, userRegex))
            return getUser();

        if (std::regex_match(environment().scriptName, audioRegex))
            return getAudio();

        if (std::regex_match(environment().scriptName, streamRegex))
            return getStream();

        if (std::regex_match(environment().scriptName, playlistRegex))
            return getPlaylist();
    /*{
        return writeMusic(test);
    }*/

        break;

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_POST:
        if (environment().scriptName.compare(usersUri) == 0)
            return addUser();

        if (environment().scriptName.compare(audiosUri) == 0)
            return addAudio();

        if (environment().scriptName.compare(playlistsUri) == 0)
            return addPlaylist();

        if (std::regex_match(environment().scriptName, tokensRegex))
            return connectUser();

        if (std::regex_match(environment().scriptName, userRegex))
            return changeUserInfo();

        if (std::regex_match(environment().scriptName, passwordRegex))
            return changePassword();

        if (std::regex_match(environment().scriptName, playlistRegex))
            return deletePlaylist();

        if (std::regex_match(environment().scriptName, playlistAudiosRegex))
            return addAudioToPlaylist();

        if (std::regex_match(environment().scriptName, playlistAudioRegex))
            return deleteAudioFromPlaylist();
    /*{
        int size = environment().findPost("file").size();
        test = std::string(environment().findPost("file").data(), size);
        setReturnCode("200 OK");
        return true;
    }*/

        break;

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_DELETE:
        if (std::regex_match(environment().scriptName, tokenRegex))
            return disconnectUser();

        break;

    default:
        break;
    }

    setReturnCode(ReturnCodes::NOT_FOUND);
    return true;
}

bool RequestHandler::writeMusic(const std::string& file)
{
    out << "Status: " << "200 OK" << "\r\n";
    out << "Content-Type: audio/mpeg\r\n\r\n";
    out << file;
    return true;
}

void RequestHandler::setReturnCode(const std::string& code)
{
    out << "Status: " << code << "\r\n";
    out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    out << code;
}

void RequestHandler::setHttpHeaders(const std::string& code)
{
    out << "Status: " << code << "\r\n";
    out << "Content-Type: application/json; charset=utf-8\r\n\r\n";
}

std::string RequestHandler::getId(int offset = 0)
{
    std::vector<std::string> parts;
    boost::split(parts, environment().scriptName, boost::is_any_of("/"));
    return parts[parts.size() - 1 - offset];
}

void RequestHandler::writeUser(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>>& writer, UserModel& user)
{
    writer.StartObject();
    writer.String("id");
    writer.String(user.id.c_str());
    writer.String("login");
    writer.String(user.login.c_str());
    writer.String("aboutyourself");
    writer.String(user.aboutYourSelf.c_str());
    writer.EndObject();
}

void RequestHandler::writeAudio(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<> > &writer, AudioModel &audio)
{
    writer.StartObject();
    writer.String("id");
    writer.String(audio.id.c_str());
    writer.String("owner");
    writer.String(audio.owner.c_str());
    writer.String("name");
    writer.String(audio.name.c_str());
    writer.String("description");
    writer.String(audio.description.c_str());
    writer.EndObject();
}

void RequestHandler::writePlaylist(rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<> > &writer, PlaylistModel &playlist, bool audios)
{
    writer.StartObject();
    writer.String("id");
    writer.String(playlist.id.c_str());
    writer.String("owner");
    writer.String(playlist.owner.c_str());
    writer.String("name");
    writer.String(playlist.name.c_str());
    writer.String("description");
    writer.String(playlist.description.c_str());

    if (audios)
    {
        writer.String("audios");
        writer.StartArray();
        int audiosCount = playlist.audios.size();

        for (int i = 0; i < audiosCount; ++i)
            writeAudio(writer, playlist.audios.at(i));

        writer.EndArray();
    }

    writer.EndObject();
}

bool RequestHandler::addUser()
{
    if (!environment().checkForPost("login") || !environment().checkForPost("password"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = environment().findPost("login").value;
    std::string password = environment().findPost("password").value;
    std::string aboutyourself = environment().checkForPost("aboutyourself") ? environment().findPost("aboutyourself").value : "";

    static std::regex loginRegex("[0-9a-zA-Z]+");

    if (!std::regex_match(login, loginRegex))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    UserModel user = DatabaseHandler::createUser(login, password, aboutyourself);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeUser(writer, user);
    setHttpHeaders(ReturnCodes::CREATED);
    out << sb.GetString();
    return true;
}

bool RequestHandler::connectUser()
{
    if (!environment().checkForPost("password"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = getId(1);
    std::string password = environment().findPost("password").value;

    UserModel user = DatabaseHandler::createToken(login, password);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("token");
    writer.String(user.token.c_str());
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::disconnectUser()
{
    std::string login = getId(2);
    std::string token = getId();
    UserModel user = DatabaseHandler::deleteToken(login, token);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("token");
    writer.String("");
    writer.EndObject();
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}


bool RequestHandler::getUser()
{
    std::string login = getId();
    UserModel user = DatabaseHandler::getUserByLogin(login);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::NOT_FOUND);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeUser(writer, user);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::getUsers()
{
    std::string loginRegex = environment().checkForGet("login") ? environment().findGet("login") : "";
    int skipCount = 0;

    if (environment().checkForGet("skipcount"))
    {
        try
        {
            skipCount = std::stoi(environment().findGet("skipcount"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    int count = 10;

    if (environment().checkForGet("count"))
    {
        try
        {
            count = std::stoi(environment().findGet("count"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    if (count <= 0 || skipCount < 0)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::vector<UserModel> users = DatabaseHandler::getUsers(loginRegex, skipCount, count);
    unsigned int usersCount = users.size();

    if (usersCount == 0)
    {
        setReturnCode(ReturnCodes::NO_CONTENT);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("users");
    writer.StartArray();

    for (unsigned int i = 0; i < usersCount; ++i)
        writeUser(writer, users.at(i));

    writer.EndArray();
    writer.String("count");
    writer.Int(usersCount);
    writer.String("nextskipcount");
    writer.Int(skipCount + usersCount);
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::changeUserInfo()
{
    if (!environment().checkForPost("token"))
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    if (!environment().checkForPost("aboutyourself"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = getId();
    std::string token = environment().findPost("token").value;
    std::string aboutYourslef = environment().findPost("aboutyourself").value;
    UserModel user = DatabaseHandler::modifyAboutYourSelf(login, token, aboutYourslef);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::FORBIDDEN);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeUser(writer, user);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::changePassword()
{
    if (!environment().checkForPost("token"))
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    if (!environment().checkForPost("oldpassword") || !environment().checkForPost("newpassword"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string token = environment().findPost("token").value;
    std::string oldPassword = environment().findPost("oldpassword").value;
    std::string newPassword = environment().findPost("newpassword").value;
    std::string login = getId(1);

    UserModel user = DatabaseHandler::modifyPassword(login, token, oldPassword, newPassword);

    if (!user.isValid)
    {
        setReturnCode(ReturnCodes::FORBIDDEN);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeUser(writer, user);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::checkUser()
{
    if (!environment().checkForPost("login") || !environment().checkForPost("token"))
        return false;

    std::string login = environment().findPost("login").value;
    std::string token = environment().findPost("token").value;
    UserModel user = DatabaseHandler::getUserByLogin(login);

    if (!user.isValid || token.compare(user.token) != 0)
            return false;

    return true;

}

bool RequestHandler::addAudio()
{
    if (!checkUser())
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    if (!environment().checkForPost("audio"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    const Fastcgipp::Http::Post<char>& audioFile = environment().findPost("audio");
    std::string owner = environment().findPost("login").value;
    std::string name = audioFile.filename;
    std::string description = environment().checkForPost("description") ? environment().findPost("description").value : "";
    AudioModel audio = DatabaseHandler::createAudio(owner, name, description, (unsigned char*) audioFile.data(), audioFile.size());

    if (!audio.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeAudio(writer, audio);
    setHttpHeaders(ReturnCodes::CREATED);
    out << sb.GetString();
    return true;
}

bool RequestHandler::getAudio()
{
    std::string audioId = getId();
    std::shared_ptr<AudioModel> audio = DatabaseHandler::getAudio(audioId, false);

    if (!audio->isValid)
    {
        setReturnCode(ReturnCodes::NOT_FOUND);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeAudio(writer, *audio);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::getAudios()
{
    std::string nameRegex = environment().checkForGet("name") ? environment().findGet("name") : "";
    std::string ownerRegex = environment().checkForGet("owner") ? environment().findGet("owner") : "";
    int skipCount = 0;

    if (environment().checkForGet("skipcount"))
    {
        try
        {
            skipCount = std::stoi(environment().findGet("skipcount"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    int count = 10;

    if (environment().checkForGet("count"))
    {
        try
        {
            count = std::stoi(environment().findGet("count"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    if (count <= 0 || skipCount < 0)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::vector<AudioModel> audios = DatabaseHandler::getAudios(nameRegex, ownerRegex, skipCount, count);
    unsigned int audiosCount = audios.size();

    if (audiosCount == 0)
    {
        setReturnCode(ReturnCodes::NO_CONTENT);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("audios");
    writer.StartArray();

    for (unsigned int i = 0; i < audiosCount; ++i)
        writeAudio(writer, audios.at(i));

    writer.EndArray();
    writer.String("count");
    writer.Int(audiosCount);
    writer.String("nextskipcount");
    writer.Int(skipCount + audiosCount);
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::getStream()
{
    std::string id = getId(1);
    std::shared_ptr<AudioModel> stream = DatabaseHandler::getAudio(id, true);

    if (!stream->isValid)
    {
        setReturnCode(ReturnCodes::NOT_FOUND);
        return true;
    }

    return writeMusic(stream->data);
}

bool RequestHandler::getPlaylists()
{
    std::string nameRegex = environment().checkForGet("name") ? environment().findGet("name") : "";
    std::string ownerRegex = environment().checkForGet("owner") ? environment().findGet("owner") : "";
    int skipCount = 0;

    if (environment().checkForGet("skipcount"))
    {
        try
        {
            skipCount = std::stoi(environment().findGet("skipcount"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    int count = 10;

    if (environment().checkForGet("count"))
    {
        try
        {
            count = std::stoi(environment().findGet("count"));
        }
        catch (...)
        {
            setReturnCode(ReturnCodes::BAD_REQUEST);
            return true;
        }
    }

    if (count <= 0 || skipCount < 0)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::vector<PlaylistModel> playlists = DatabaseHandler::getPlaylists(nameRegex, ownerRegex, skipCount, count);
    unsigned int playlistsCount = playlists.size();

    if (playlistsCount == 0)
    {
        setReturnCode(ReturnCodes::NO_CONTENT);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("playlists");
    writer.StartArray();

    for (unsigned int i = 0; i < playlistsCount; ++i)
        writePlaylist(writer, playlists.at(i), false);

    writer.EndArray();
    writer.String("count");
    writer.Int(playlistsCount);
    writer.String("nextskipcount");
    writer.Int(skipCount + playlistsCount);
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::addPlaylist()
{
    if (!checkUser())
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    if (!environment().checkForPost("name"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string owner = environment().findPost("login").value;
    std::string name = environment().findPost("name").value;
    std::string description = environment().checkForPost("description") ? environment().findPost("description").value : "";
    std::string id = environment().checkForPost("id") ? environment().findPost("id").value : "";
    PlaylistModel playlist = DatabaseHandler::createPlaylist(name, id, owner, description);

    if (!playlist.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writePlaylist(writer, playlist, true);
    setHttpHeaders(ReturnCodes::CREATED);
    out << sb.GetString();
    return true;
}

bool RequestHandler::getPlaylist()
{
    std::string playlistId = getId();
    PlaylistModel playlist = DatabaseHandler::getPlaylist(playlistId);

    if (!playlist.isValid)
    {
        setReturnCode(ReturnCodes::NOT_FOUND);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writePlaylist(writer, playlist, true);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::deletePlaylist()
{
    if (!checkUser())
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    std::string playlistId = getId();
    std::string login = environment().findPost("login").value;
    PlaylistModel playlist = DatabaseHandler::deletePlaylist(playlistId, login);

    if (!playlist.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("id");
    writer.String(playlist.id.c_str());
    writer.EndObject();
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::addAudioToPlaylist()
{
    if (!checkUser())
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    if (!environment().checkForPost("audioid"))
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string audioId = environment().findPost("audioid").value;
    std::string playlistId = getId(1);
    std::string login = environment().findPost("login").value;
    PlaylistModel playlist = DatabaseHandler::addAudioToPlaylist(audioId, playlistId, login);

    if (!playlist.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writePlaylist(writer, playlist, true);
    setHttpHeaders(ReturnCodes::CREATED);
    out << sb.GetString();
    return true;
}

bool RequestHandler::deleteAudioFromPlaylist()
{
    if (!checkUser())
    {
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    std::string audioId = getId();
    std::string playlistId = getId(2);
    std::string login = environment().findPost("login").value;
    PlaylistModel playlist = DatabaseHandler::deleteAudioFromPlaylist(audioId, playlistId, login);

    if (!playlist.isValid)
    {
        setReturnCode(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writePlaylist(writer, playlist, true);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}
