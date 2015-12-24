#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <string>
#include <fastcgi++/request.hpp>

#include <stdlib.h>

class RequestHandler: public Fastcgipp::Request<char>
{
private:
    void setError(std::string error);
    void setHttpHeaders(std::string code);

    std::string getId(int offset);
    bool addUser();
    bool connectUser();
    bool disconnectUser();
    bool getUser();

public:
    bool response();
};

#endif
