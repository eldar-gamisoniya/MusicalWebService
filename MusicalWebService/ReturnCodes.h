#ifndef RETURNCODES_H
#define RETURNCODES_H

#include <string>

class ReturnCodes
{
public:
    static const std::string NOT_FOUND;
    static const std::string BAD_REQUEST;
    static const std::string CREATED;
    static const std::string OK;
    static const std::string NO_CONTENT;
    static const std::string UNAUTHORIZED;
    static const std::string FORBIDDEN;
};

#endif // RETURNCODES_H
