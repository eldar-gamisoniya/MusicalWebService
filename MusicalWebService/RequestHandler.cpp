#include "RequestHandler.h"
#include "UserModel.h"
#include <regex>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "DatabaseHandler.h"
#include "ReturnCodes.h"

bool RequestHandler::response()
{
    static std::regex getUserRegex("/api/v1/users/[0-9a-z]+");
    static std::regex tokenRegex("/api/v1/users/[0-9a-z]+/token");

    static char usersUri[] = "/api/v1/users";

    switch (environment().requestMethod)
    {
    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_GET:
        if (std::regex_match(environment().scriptName, getUserRegex))
            return getUser();

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_POST:
        if (environment().scriptName.compare(usersUri) == 0)
            return addUser();

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_PUT:
        if (std::regex_match(environment().scriptName, tokenRegex))
            return connectUser();

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_DELETE:
        if (std::regex_match(environment().scriptName, tokenRegex))
            return disconnectUser();

    default:
        break;
    }

    setError(ReturnCodes::NOT_FOUND);
    return true;
}

bool RequestHandler::writeMusic(const char* file, int size)
{
    int written = 0;
    const char* current = file;

    out << "Status: " << "200 OK" << "\r\n";
    out << "Content-Type: audio/mpeg\r\n\r\n";

    while (written < size)
    {
        int charsToWrite = strlen(current);

        if (charsToWrite > 0)
        {
            out << current;
            written += charsToWrite;
            current += charsToWrite;
        }
        else
        {
            out << (char) 0;
            written += 1;
            current += 1;
        }
    }

    return true;
}

void RequestHandler::setError(const std::string& error)
{
    out << "Status: " << error << "\r\n";
    out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    out << error;
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

bool RequestHandler::addUser()
{
    if (!environment().checkForPost("login") || !environment().checkForPost("password"))
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = environment().findPost("login").value;
    std::string password = environment().findPost("password").value;
    std::string aboutyourself;

    if (environment().checkForPost("aboutyourself"))
        aboutyourself = environment().findPost("aboutyourself").value;

    UserModel user = DatabaseHandler::createUser(login, password, aboutyourself);

    if (!user.isValid)
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("userid");
    writer.String(user.id.c_str());
    writer.String("login");
    writer.String(user.login.c_str());
    writer.String("aboutyourself");
    writer.String(user.aboutYourSelf.c_str());
    writer.EndObject();

    setHttpHeaders(ReturnCodes::CREATED);
    out << sb.GetString();
    return true;
}

bool RequestHandler::connectUser()
{
    if (!environment().checkForPost("password"))
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = getId(1);
    std::string password = environment().findPost("password").value;

    UserModel user = DatabaseHandler::connect(login, password);

    if (!user.isValid)
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("accesstoken");
    writer.String(user.token.c_str());
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}

bool RequestHandler::disconnectUser()
{
    if (!environment().checkForPost("password"))
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    std::string login = getId(1);
    std::string password = environment().findPost("password").value;

    UserModel user = DatabaseHandler::disconnect(login, password);

    if (!user.isValid)
    {
        setError(ReturnCodes::BAD_REQUEST);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("result");
    writer.String("disconnected");
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
        setError(ReturnCodes::NOT_FOUND);
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("login");
    writer.String(user.login.c_str());
    writer.String("aboutyourself");
    writer.String(user.aboutYourSelf.c_str());
    writer.EndObject();

    setHttpHeaders(ReturnCodes::OK);
    out << sb.GetString();
    return true;
}
