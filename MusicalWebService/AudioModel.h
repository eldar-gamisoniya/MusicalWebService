#ifndef AUDIOMODEL_H
#define AUDIOMODEL_H

#include <string>

class AudioModel
{
public:
    std::string owner;
    std::string description;
    std::string id;
    std::string name;
    std::string data;
    long timestamp;
    bool isValid;
};

#endif // MUSICMODEL_H
