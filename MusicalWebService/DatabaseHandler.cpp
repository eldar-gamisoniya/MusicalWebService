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

std::string DatabaseHandler::newUuid()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    return boost::lexical_cast<std::string>(uuid);
}

std::string DatabaseHandler::elementToString(const bsoncxx::document::element& el)
{
    return std::string(el.get_utf8().value);
}

std::string DatabaseHandler::elementToStringId(const bsoncxx::document::element& el)
{
    return el.get_oid().value.to_string();
}

long DatabaseHandler::elementToLong(const bsoncxx::document::element& el)
{
    return el.get_int64().value;
}

long DatabaseHandler::elementToDate(const bsoncxx::document::element& el)
{
    return el.get_date().value;
}

long DatabaseHandler::currentTimestamp()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
}

std::vector<UserModel> DatabaseHandler::getUsers(const std::string& loginRegex, const int skipCount, const int count)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    document filter;

    if (loginRegex.size() > 0)
        filter << "login" << open_document << "$regex" << loginRegex << close_document;

    mongocxx::options::find options;
    document order;
    order << "timestamp" << 1 << "_id" << 1;
    options.sort(order.view());
    options.limit(count);
    options.skip(skipCount);

    auto cursor = db["Users"].find(filter.view(), options);

    std::vector<UserModel> v;

    for (auto&& doc: cursor)
    {
        UserModel model;
        model.login = elementToString(doc["login"]);
        model.id = elementToStringId(doc["_id"]);
        model.password = elementToString(doc["password"]);
        model.aboutYourSelf = elementToString(doc["aboutyourself"]);
        model.token = elementToString(doc["token"]);
        model.timestamp = elementToDate(doc["timestamp"]);
        model.isValid = true;
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
            << "timestamp" << bsoncxx::types::b_date(timestamp)
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
    userModel.timestamp = timestamp;
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

    userModel.id = elementToStringId(user["_id"]);
    userModel.login = login;
    userModel.password = elementToString(user["password"]);
    userModel.aboutYourSelf = elementToString(user["aboutyourself"]);
    userModel.token = elementToString(user["token"]);
    userModel.timestamp = elementToDate(user["timestamp"]);
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



AudioModel DatabaseHandler::createAudio(const std::string& owner, const std::string& name, const std::string& description,
                              const unsigned char* data, const int size)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    long timestamp  = currentTimestamp();

    bsoncxx::types::b_binary b_data;
    b_data.bytes = data;
    b_data.size = size;

    auto audio_doc = document{}
            << "owner" << owner
            << "name" << name
            << "description" << description
            << "data" << b_data
            << "timestamp" << bsoncxx::types::b_date(timestamp)
            << finalize;

    AudioModel audioModel;

    try
    {
        auto res = db["Audios"].insert_one(std::move(audio_doc));
        if (res->result().inserted_count() != 1)
        {
            audioModel.isValid = false;
            return audioModel;
        }
        audioModel.id = res->inserted_id().get_oid().value.to_string();
    }
    catch(...)
    {
        audioModel.isValid = false;
        return audioModel;
    }
    audioModel.isValid = true;
    audioModel.owner = owner;
    audioModel.name = name;
    audioModel.description = description;
    audioModel.timestamp = timestamp;
    return audioModel;
}

std::shared_ptr<AudioModel> DatabaseHandler::getAudio(const std::string& id, bool getData)
{
    mongocxx::client& conn = connection.getConnection();

    auto db = conn["MusicalWebService"];

    std::shared_ptr<AudioModel> audioModel(new AudioModel);
    bsoncxx::document::view audio;

    mongocxx::options::find options;

    if (!getData)
    {
        document projection;
        projection << "data" << 0 << finalize;
        options.projection(projection.view());
    }

    auto cursor = db["Audios"].find(document{} << "_id" << bsoncxx::oid(id) <<finalize, options);
    bool iterated = false;
    for (auto&& doc: cursor)
    {
        iterated = true;
        audio = doc;
        break;
    }

    if (!iterated)
    {
        audioModel->isValid = false;
        return audioModel;
    }

    audioModel->id = id;
    audioModel->owner = elementToString(audio["owner"]);
    audioModel->name = elementToString(audio["name"]);
    audioModel->description = elementToString(audio["description"]);
    audioModel->timestamp = elementToDate(audio["timestamp"]);
    if (getData)
    {
        const bsoncxx::types::b_binary& data = audio["data"].get_binary();
        audioModel->data = std::string((const char*)data.bytes, data.size);
    }
    audioModel->isValid = true;
    return audioModel;
}

std::vector<AudioModel> DatabaseHandler::getAudios(const std::string& nameRegex, const std::string& ownerRegex,
                                         const int skipCount, const int count)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    document filter;

    if (nameRegex.size() > 0)
        filter << "name" << open_document << "$regex" << nameRegex << close_document;
    if (ownerRegex.size() > 0)
        filter << "owner" << open_document << "$regex" << ownerRegex << close_document;

    mongocxx::options::find options;
    document order;
    order << "timestamp" << 1 << "_id" << 1;
    options.sort(order.view());
    document projection;;
    projection << "data" << 0 << finalize;
    options.projection(projection.view());
    options.limit(count);
    options.skip(skipCount);

    auto cursor = db["Audios"].find(filter.view(), options);

    std::vector<AudioModel> v;

    for (auto&& doc: cursor)
    {
        AudioModel audioModel;
        audioModel.id = elementToStringId(doc["_id"]);
        audioModel.owner = elementToString(doc["owner"]);
        audioModel.name = elementToString(doc["name"]);
        audioModel.description = elementToString(doc["description"]);
        audioModel.timestamp = elementToDate(doc["timestamp"]);
        audioModel.isValid = true;
        v.push_back(std::move(audioModel));
    }

    return v;
}

PlaylistModel DatabaseHandler::getPlaylist(const std::string& id)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    PlaylistModel playlistModel;
    bsoncxx::document::view playlist;

    auto cursor = db["Playlists"].find(document{} << "_id" << bsoncxx::oid(id) <<finalize);
    bool iterated = false;
    for (auto&& doc: cursor)
    {
        iterated = true;
        playlist = doc;
        break;
    }

    if (!iterated)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    playlistModel.id = id;
    playlistModel.description = elementToString(playlist["description"]);
    playlistModel.name = elementToString(playlist["name"]);
    playlistModel.owner = elementToString(playlist["owner"]);
    playlistModel.timestamp = elementToDate(playlist["timestamp"]);

    auto arr = playlist["audios"].get_array().value;

    for (auto i = arr.cbegin(); i != arr.cend(); i++)
    {
        AudioModel audioModel;
        auto doc = i->get_document().value;
        audioModel.name = elementToString(doc["name"]);
        audioModel.owner = elementToString(doc["owner"]);
        audioModel.id = elementToStringId(doc["_id"]);
        audioModel.description = elementToString(doc["description"]);
        playlistModel.audios.push_back(std::move(audioModel));
    }

    playlistModel.isValid = true;
    return playlistModel;
}

PlaylistModel DatabaseHandler::createPlaylist(const std::string& name, const std::string& copyingId,
                                    const std::string& owner, const std::string& description)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    long timestamp  = currentTimestamp();

    PlaylistModel playlistModel;

    bsoncxx::document::value newPlaylist = document{} << "owner" << owner
                << "name" << name
                << "description" << description
                << "audios" << open_array <<
                   [&](bsoncxx::builder::stream::array_context<> arr) {
                        if (copyingId.size() > 0)
                        {
                            playlistModel = getPlaylist(copyingId);
                            unsigned int size = playlistModel.audios.size();

                            for (unsigned int i = 0; i < size; ++i)
                            {
                                arr << open_document
                                             << "_id" << bsoncxx::oid(playlistModel.audios[i].id)
                                             << "owner" << playlistModel.audios[i].owner
                                             << "name" << playlistModel.audios[i].name
                                             << "description" << playlistModel.audios[i].description
                                             << close_document;
                            }
                        }
                    }
                << close_array
                << "timestamp" << bsoncxx::types::b_date(timestamp) <<finalize;

    try
    {
        auto res = db["Playlists"].insert_one(std::move(newPlaylist));

        if (res->result().inserted_count() != 1)
        {
            playlistModel.isValid = false;
            return playlistModel;
        }

        playlistModel.id = res->inserted_id().get_oid().value.to_string();
    }
    catch(...)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    playlistModel.isValid = true;
    playlistModel.owner = owner;
    playlistModel.name = name;
    playlistModel.description = description;
    playlistModel.timestamp = timestamp;
    return playlistModel;
}

std::vector <PlaylistModel> DatabaseHandler::getPlaylists(const std::string& nameRegex, const std::string& ownerRegex,
                                                const int skipCount, const int count)
{
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    document filter;

    if (nameRegex.size() > 0)
        filter << "name" << open_document << "$regex" << nameRegex << close_document;
    if (ownerRegex.size() > 0)
        filter << "owner" << open_document << "$regex" << ownerRegex << close_document;

    mongocxx::options::find options;
    document order;
    order << "timestamp" << 1 << "_id" << 1;
    options.sort(order.view());
    document projection;;
    projection << "audios" << 0 << finalize;
    options.projection(projection.view());
    options.limit(count);
    options.skip(skipCount);

    auto cursor = db["Playlists"].find(filter.view(), options);

    std::vector<PlaylistModel> v;

    for (auto&& doc: cursor)
    {
        PlaylistModel playlistModel;
        playlistModel.id = elementToStringId(doc["_id"]);
        playlistModel.owner = elementToString(doc["owner"]);
        playlistModel.name = elementToString(doc["name"]);
        playlistModel.description = elementToString(doc["description"]);
        playlistModel.timestamp = elementToDate(doc["timestamp"]);
        playlistModel.isValid = true;
        v.push_back(std::move(playlistModel));
    }

    return v;
}

PlaylistModel DatabaseHandler::addAudioToPlaylist(const std::string& audioId, const std::string& playlistId,
                                        const std::string& owner)
{
    PlaylistModel playlistModel = getPlaylist(playlistId);
    std::shared_ptr<AudioModel> audioModel = getAudio(audioId, false);

    if (!playlistModel.isValid || !audioModel->isValid || owner.compare(playlistModel.owner) != 0)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    playlistModel.audios.push_back(*audioModel);
    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];

    bsoncxx::builder::stream::document filterPlaylist, updatePlaylist;
    filterPlaylist << "_id"
                   << bsoncxx::oid(playlistId);
    updatePlaylist << "$set" << open_document << "audios" << open_array <<
                        [&](bsoncxx::builder::stream::array_context<> arr) {
                            unsigned int size = playlistModel.audios.size();

                            for (unsigned int i = 0; i < size; ++i)
                            {
                                arr << open_document
                                    << "_id" << bsoncxx::oid(playlistModel.audios[i].id)
                                    << "owner" << playlistModel.audios[i].owner
                                    << "name" << playlistModel.audios[i].name
                                    << "description" << playlistModel.audios[i].description
                                    << close_document;
                            }
                        }
                   << close_array << close_document;

    try
    {
        auto res = db["Playlists"].update_one(filterPlaylist.view(), updatePlaylist.view());

        if (res->modified_count() != 1)
        {
            playlistModel.isValid = false;
            return playlistModel;
        }
    }
    catch(...)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    return playlistModel;
}

PlaylistModel DatabaseHandler::deleteAudioFromPlaylist(const std::string& audioId, const std::string& playlistId,
                                             const std::string& owner)
{
    PlaylistModel playlistModel = getPlaylist(playlistId);

    if (!playlistModel.isValid || owner.compare(playlistModel.owner) != 0)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];
    bool deleted = false;
    unsigned int deletedIndex;

    bsoncxx::builder::stream::document filterPlaylist, updatePlaylist;
    filterPlaylist << "_id"
                   << bsoncxx::oid(playlistId);
    updatePlaylist << "$set" << open_document << "audios" << open_array <<
                        [&](bsoncxx::builder::stream::array_context<> arr) {
                            unsigned int size = playlistModel.audios.size();

                            for (unsigned int i = 0; i < size; ++i)
                            {

                                if (audioId.compare(playlistModel.audios[i].id) != 0 || deleted)
                                {
                                    arr << open_document
                                        << "_id" << bsoncxx::oid(playlistModel.audios[i].id)
                                        << "owner" << playlistModel.audios[i].owner
                                        << "name" << playlistModel.audios[i].name
                                        << "description" << playlistModel.audios[i].description
                                        << close_document;
                                }
                                else if (!deleted)
                                {
                                    deleted = true;
                                    deletedIndex = i;
                                }
                            }
                        }
                   << close_array << close_document;

    if (!deleted)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    try
    {
        auto res = db["Playlists"].update_one(filterPlaylist.view(), updatePlaylist.view());

        if (res->modified_count() != 1)
        {
            playlistModel.isValid = false;
            return playlistModel;
        }
    }
    catch(...)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    playlistModel.audios.erase(playlistModel.audios.begin() + deletedIndex);
    return playlistModel;
}

PlaylistModel DatabaseHandler::deletePlaylist(const std::string& id, const std::string& owner)
{
    PlaylistModel playlistModel = getPlaylist(id);

    if (!playlistModel.isValid || owner.compare(playlistModel.owner) != 0)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    mongocxx::client& conn = connection.getConnection();
    auto db = conn["MusicalWebService"];
    auto deletePlaylist = document{}
            << "_id"
            << bsoncxx::oid(id)
            << finalize;

    try
    {
        auto res = db["Playlists"].delete_one(std::move(deletePlaylist));

        if (res->deleted_count() != 1)
        {
            playlistModel.isValid = false;
            return playlistModel;
        }
    }
    catch(...)
    {
        playlistModel.isValid = false;
        return playlistModel;
    }

    return playlistModel;
}
