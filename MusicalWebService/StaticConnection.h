#ifndef STATICCONNECTION_H
#define STATICCONNECTION_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class StaticConnection
{
private:
    mongocxx::instance instance;
    mongocxx::client conn;

public:
    StaticConnection();
    mongocxx::client& getConnection();
};

#endif // STATICCONNECTION_H
