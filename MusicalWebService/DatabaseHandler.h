#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include "UserModel.h"
#include "AudioModel.h"
#include "PlaylistModel.h"
#include "StaticConnection.h"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <vector>
#include <memory>


class DatabaseHandler
{
private:
    std::string newUuid();
    std::string elementToString(const bsoncxx::document::element& el);
    std::string elementToStringId(const bsoncxx::document::element& el);
    long elementToLong(const bsoncxx::document::element& el);
    long elementToDate(const bsoncxx::document::element& el);
    long currentTimestamp();
    StaticConnection connection;

public:
    //User operations
    std::vector<UserModel> getUsers(const std::string& loginRegex, const int skipCount, const int count);
    UserModel createUser(const std::string& login,const std::string& password,
                                const std::string& aboutYourSelf);
    UserModel modifyAboutYourSelf(const std::string& login, const std::string& token,
                                         const std::string& aboutYourSelf);
    UserModel modifyPassword(const std::string& login, const std::string& token,
                                    const std::string& oldPassword, const std::string& newPassword);
    UserModel getUserByLogin(const std::string& login);
    UserModel createToken(const std::string& login, const std::string& password);
    UserModel deleteToken(const std::string& login, const std::string& token);

    //Audio operations
    AudioModel createAudio(const std::string& owner, const std::string& name, const std::string& description,
                                  const unsigned char* data, const int size);
    std::shared_ptr<AudioModel> getAudio(const std::string& id, bool getData);
    std::vector<AudioModel> getAudios(const std::string& nameRegex, const std::string& ownerRegex,
                                             const int skipCount, const int count);

    //Playlist operations
    std::vector <PlaylistModel> getPlaylists(const std::string& nameRegex, const std::string& ownerRegex,
                                                    const int skipCount, const int count);
    PlaylistModel getPlaylist(const std::string& id);
    PlaylistModel addAudioToPlaylist(const std::string& audioId, const std::string& playlistId,
                                            const std::string& owner);
    PlaylistModel deleteAudioFromPlaylist(const std::string& audioId, const std::string& playlistId,
                                                 const std::string& owner);
    PlaylistModel createPlaylist(const std::string& name, const std::string& copyingId,
                                        const std::string& owner, const std::string& description);
    PlaylistModel deletePlaylist(const std::string& id, const std::string& owner);
};
#endif
