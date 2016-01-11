#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <string>
#include <vector>

#include "AudioModel.h"

class PlaylistModel
{
public:
    std::string owner;
    std::string description;
    std::string id;
    std::string name;
    std::vector<AudioModel> audios;
    long timestamp;
    bool isValid;
};

#endif // PLAYLISTMODEL_H
