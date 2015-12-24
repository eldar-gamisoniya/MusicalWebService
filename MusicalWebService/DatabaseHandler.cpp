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

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

std::string DatabaseHandler::newUuid()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    return boost::lexical_cast<std::string>(uuid);
}

std::string DatabaseHandler::elementToString(bsoncxx::document::element el)
{
    std::string tmp = bsoncxx::to_json(el.get_value());
    return tmp.substr(1, tmp.size() - 2);
}

UserModel DatabaseHandler::createUser(std::string login, std::string password, std::string aboutYourSelf)
{
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

    auto db = conn["MusicalWebService"];

    auto user_doc = document{}
            << "login" << login
            << "password" << password
            << "aboutyourself" << aboutYourSelf
            << "token" << ""
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
        userModel.id = bsoncxx::to_json(res->inserted_id());
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

UserModel DatabaseHandler::getUserByLogin(std::string login)
{
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

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
    userModel.password = elementToString(user["password"]);
    userModel.aboutYourSelf = elementToString(user["aboutyourself"]);
    userModel.token = elementToString(user["token"]);
    userModel.isValid = true;
    return userModel;
}

UserModel DatabaseHandler::modifyToken(std::string login, std::string password, std::string newToken)
{
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

    auto db = conn["MusicalWebService"];

    UserModel userModel = getUserByLogin(login);

    if (!userModel.isValid)
        return userModel;

    if (password.compare(userModel.password) != 0)
    {
        userModel.isValid = false;
        return userModel;
    }

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

    userModel.token = newToken;
    userModel.isValid = true;
    return userModel;

}

UserModel DatabaseHandler::connect(std::string login, std::string password)
{
    return modifyToken(login, password, newUuid());
}

UserModel DatabaseHandler::disconnect(std::string login, std::string password)
{
    return modifyToken(login, password, "");
}
