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

bool RequestHandler::response()
{
    /*GenericDocument<UTF16<> > d;
    //Document d;

    d.SetObject();

    Document::AllocatorType& allocator = d.GetAllocator();

    wstring str = L"world";

    d.AddMember(L"hello", StringRef(str.c_str()), allocator);
    d.AddMember(L"uri", StringRef(environment().scriptName.c_str()), allocator);
    d.AddMember(L"method", environment().requestMethod, allocator);

    StringBuffer sb;
    //PrettyWriter<StringBuffer> writer(sb);
    PrettyWriter< StringBuffer, UTF16<> >  writerUTF16(sb);
    //d.Accept(writer);
    d.Accept(writerUTF16);

    //writerUTF16.StartObject();

    //writer.String("uri:");
    //writer.String("123");
    //wchar_t c[] = L"c";
    //writerUTF16.String();
    //writerUTF16.String(c);

    //writer.EndObject();

    out << "Status: 404 Not found\r\n\r\n";
    out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    out << sb.GetString();

    err << "Hello apache error log";*/

    /*rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF16<>> writer(sb);
    writer.StartObject();
    writer.String(L"uri:");
    writer.String(environment().scriptName.c_str());
    if(environment().posts.size())
    {
       for(Fastcgipp::Http::Environment<wchar_t>::Posts::const_iterator it=environment().posts.begin();
it!=environment().posts.end(); ++it)
       {
           writer.String(L"first:");
           writer.String(it->first.c_str());
           writer.String(L"second:");
           writer.String(it->second.value.c_str());
       }
    }
    writer.EndObject();
    out << "Status: 200 OK\r\n\r\n";
    out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    out <<sb.GetString();*/


    std::regex getUserRegex("/api/v1/users/[0-9a-z]+");
    std::regex connectRegex("/api/v1/users/[0-9a-z]+/connect");
    std::regex disconnectRegex("/api/v1/users/[0-9a-z]+/disconnect");

    switch (environment().requestMethod)
    {
    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_GET:
        if (std::regex_match(environment().scriptName, getUserRegex))
            return getUser();

        break;

    case Fastcgipp::Http::RequestMethod::HTTP_METHOD_POST:
        if (environment().scriptName.compare("/api/v1/users") == 0)
            return addUser();

        if (std::regex_match(environment().scriptName, connectRegex))
            return connectUser();

        if (std::regex_match(environment().scriptName, disconnectRegex))
            return disconnectUser();

        break;

    default:
        break;
    }

    setError("404 Not Found");
    return true;
}

void RequestHandler::setError(std::string error)
{
    out << "Status: " <<error <<"\r\n";
    out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
    out << error;
}

void RequestHandler::setHttpHeaders(std::string code)
{
    out << "Status: " <<code <<"\r\n";
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
        setError("400 Bad Request");
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
        setError("400 Bad Request");
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

    setHttpHeaders("201 Created");
    out << sb.GetString();
    return true;
}

bool RequestHandler::connectUser()
{
    if (!environment().checkForPost("password"))
    {
        setError("400 Bad Request");
        return true;
    }

    std::string login = getId(1);
    std::string password = environment().findPost("password").value;

    UserModel user = DatabaseHandler::createToken(login, password);

    if (!user.isValid)
    {
        setError("400 Bad Request");
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("accesstoken");
    writer.String(user.token.c_str());
    writer.EndObject();

    setHttpHeaders("200 OK");
    out << sb.GetString();
    return true;
}

bool RequestHandler::disconnectUser()
{
    if (!environment().checkForPost("password"))
    {
        setError("400 Bad Request");
        return true;
    }

    std::string login = getId(1);
    std::string password = environment().findPost("password").value;

    UserModel user = DatabaseHandler::deleteToken(login, password);

    if (!user.isValid)
    {
        setError("400 Bad Request");
        return true;
    }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>> writer(sb);
    writer.StartObject();
    writer.String("result");
    writer.String("disconnected");
    writer.EndObject();

    setHttpHeaders("200 OK");
    out << sb.GetString();
    return true;
}


bool RequestHandler::getUser()
{
    std::string login = getId();

    UserModel user = DatabaseHandler::getUserByLogin(login);
    if (!user.isValid)
    {
        setError("404 Not Found");
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

    setHttpHeaders("200 OK");
    out << sb.GetString();
    return true;
}
