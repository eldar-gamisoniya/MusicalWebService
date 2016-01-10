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

    static char usersUri[] = "/api/v1/users";

    //static std::string test;

    switch (environment().requestMethod)
    {
    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_GET:
        if (std::regex_match(environment().scriptName, userRegex))
            return getUser();

        if (environment().scriptName.compare(usersUri) == 0)
            return getUsers();

    /*{
        return writeMusic(test);
    }*/

        break;

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_POST:
        if (environment().scriptName.compare(usersUri) == 0)
            return addUser();

        if (std::regex_match(environment().scriptName, tokensRegex))
            return connectUser();

        if (std::regex_match(environment().scriptName, userRegex))
            return changeUserInfo();

        if (std::regex_match(environment().scriptName, passwordRegex))
            return changePassword();

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

    setReturnCode(ReturnCodes::OK);
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
    {
        UserModel user = users.at(i);
        writeUser(writer, user);
    }

    writer.EndArray();
    writer.String("count");
    writer.Int(usersCount);
    writer.String("nextSkipCount");
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
        setReturnCode(ReturnCodes::UNAUTHORIZED);
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
        setReturnCode(ReturnCodes::UNAUTHORIZED);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writeUser(writer, user);
    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}
