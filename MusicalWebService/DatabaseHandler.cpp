#include "DatabaseHandler.h"

#include <bsoncxx/types.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <chrono>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

StaticConnection DatabaseHandler::connection;

std::string DatabaseHandler::newUuid()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    return boost::lexical_cast<std::string>(uuid);
}

std::string DatabaseHandler::stringElementToString(bsoncxx::document::element el)
{
    return std::string(el.get_value().get_utf8().value);
}

long DatabaseHandler::currentTimestamp()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
}

std::vector<UserModel> DatabaseHandler::getUsers(const std::string& loginRegex,const long timestampFrom,
                                                const long count)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    document filter;
    filter << "timestamp" << open_document << "$gt" << timestampFrom << close_document;
    if (loginRegex.size() > 0)
        filter << "login" << open_document << "$regex" << loginRegex << close_document;

    mongocxx::options::find options;
    document order;
    order << "timestamp" << 1 << "login" << 1;
    options.sort(order.view());
    options.limit(count);

    auto cursor = db["Users"].find(filter.view(), options);

    std::vector<UserModel> v;

    for (auto&& doc: cursor)
    {
        UserModel model;
        model.login = stringElementToString(doc["login"]);
        model.password = stringElementToString(doc["password"]);
        model.aboutYourSelf = stringElementToString(doc["aboutyourself"]);
        model.token = stringElementToString(doc["token"]);
        v.push_back(std::move(model));
    }

    return v;
}

UserModel DatabaseHandler::createUser(const std::string& login,const std::string& password,
                                      const std::string& aboutYourSelf)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    long timestamp  = currentTimestamp();

    auto user_doc = document{}
            << "login" << login
            << "password" << password
            << "aboutyourself" << aboutYourSelf
            << "token" << ""
            << "timestamp" << timestamp
            << finalize;

    UserModel userModel;

    try
    {
        auto res = db["Users"].insert_one(std::move(user_doc));
        if (res->result().inserted_count() != 1)
        {
            userModel.isValid = false;
            return userModel;
        }
        userModel.id = res->inserted_id().get_oid().value.to_string();
    }
    catch(...)
    {
        userModel.isValid = false;
        return userModel;
    }
    userModel.isValid = true;
    userModel.login = login;
    userModel.password = password;
    userModel.aboutYourSelf = aboutYourSelf;
    return userModel;
}

UserModel DatabaseHandler::getUserByLogin(const std::string& login)
{
    mongocxx::client& conn = connection.getConnection();

    auto db = conn["MusicalWebService"];

    UserModel userModel;
    bsoncxx::document::view user;

    auto cursor = db["Users"].find(document{} << "login" << login <<finalize);
    bool iterated = false;
    for (auto&& doc: cursor)
    {
        iterated = true;
        user = doc;
        break;
    }

    if (!iterated)
    {
        userModel.isValid = false;
        return userModel;
    }

    userModel.login = login;
    userModel.password = stringElementToString(user["password"]);
    userModel.aboutYourSelf = stringElementToString(user["aboutyourself"]);
    userModel.token = stringElementToString(user["token"]);
    userModel.isValid = true;
    return userModel;
}

UserModel DatabaseHandler::createToken(const std::string& login, const std::string& password)
{
    mongocxx::client& conn = connection.getConnection();

    UserModel userModel = getUserByLogin(login);

    auto db = conn["MusicalWebService"];

    if (!userModel.isValid)
        return userModel;

    if (password.compare(userModel.password) != 0)
    {
        userModel.isValid = false;
        return userModel;
    }

    std::string newToken = newUuid();

    document filter;
    filter << "login" << login;
    document update;
    update << "$set" << open_document << "token" << newToken << close_document;

    try
    {
        auto res = db["Users"].update_one(filter.view(), update.view());
        if (res->result().modified_count() != 1)
        {
            userModel.isValid = false;
            return userModel;
        }
    }
    catch (...)
    {
        userModel.isValid = false;
        return userModel;
    }

    userModel.token = std::move(newToken);
    userModel.isValid = true;
    return userModel;
}

UserModel DatabaseHandler::deleteToken(const std::string& login, const std::string& token)
{
    mongocxx::client& conn = connection.getConnection();

    UserModel userModel = getUserByLogin(login);

    auto db = conn["MusicalWebService"];

    if (!userModel.isValid)
        return userModel;

    if (token.compare(userModel.token) != 0)
    {
        userModel.isValid = false;
        return userModel;
    }

    std::string newToken = "";

    document filter;
    filter << "login" << login;
    document update;
    update << "$set" << open_document << "token" << newToken << close_document;

    try
    {
        auto res = db["Users"].update_one(filter.view(), update.view());
        if (res->result().modified_count() != 1)
        {
            userModel.isValid = false;
            return userModel;
        }
    }
    catch (...)
    {
        userModel.isValid = false;
        return userModel;
    }

    userModel.token = std::move(newToken);
    userModel.isValid = true;
    return userModel;
}

UserModel DatabaseHandler::modifyAboutYourSelf(const std::string& login, const std::string& token,
                                         const std::string& aboutYourSelf)
{
    mongocxx::client& conn = connection.getConnection();

    UserModel userModel = getUserByLogin(login);

    auto db = conn["MusicalWebService"];

    if (!userModel.isValid)
        return userModel;

    if (token.compare(userModel.token) != 0)
    {
        userModel.isValid = false;
        return userModel;
    }

    document filter;
    filter << "login" << login;
    document update;
    update << "$set" << open_document << "aboutyourself" << aboutYourSelf << close_document;

    try
    {
        auto res = db["Users"].update_one(filter.view(), update.view());
        if (res->result().modified_count() != 1)
        {
            userModel.isValid = false;
            return userModel;
        }
    }
    catch (...)
    {
        userModel.isValid = false;
        return userModel;
    }

    userModel.aboutYourSelf = aboutYourSelf;
    userModel.isValid = true;
    return userModel;
}

UserModel DatabaseHandler::modifyPassword(const std::string& login, const std::string& token,
                                          const std::string& oldPassword, const std::string& newPassword)
{
    mongocxx::client& conn = connection.getConnection();

    UserModel userModel = getUserByLogin(login);

    auto db = conn["MusicalWebService"];

    if (!userModel.isValid)
        return userModel;

    if (oldPassword.compare(userModel.password) != 0 || token.compare(userModel.token) != 0)
    {
        userModel.isValid = false;
        return userModel;
    }

    document filter;
    filter << "login" << login;
    document update;
    update << "$set" << open_document << "password" << newPassword << close_document;

    try
    {
        auto res = db["Users"].update_one(filter.view(), update.view());
        if (res->result().modified_count() != 1)
        {
            userModel.isValid = false;
            return userModel;
        }
    }
    catch (...)
    {
        userModel.isValid = false;
        return userModel;
    }

    userModel.password = newPassword;
    userModel.isValid = true;
    return userModel;
}
