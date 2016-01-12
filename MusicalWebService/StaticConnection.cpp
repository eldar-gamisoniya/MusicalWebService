#include "StaticConnection.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

StaticConnection::StaticConnection()
{
    instance = mongocxx::instance{};
    conn = mongocxx::client{mongocxx::uri{"mongodb://mongo0.com:27017,mongo1.com:27017/?replicaSet=rs0&readPreference=nearest"}};
}

mongocxx::client& StaticConnection::getConnection()
{
    return conn;
}
