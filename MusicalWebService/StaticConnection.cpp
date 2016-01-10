#include "StaticConnection.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

StaticConnection::StaticConnection()
{
    instance = mongocxx::instance{};
    conn = mongocxx::client{mongocxx::uri{}};
}

mongocxx::client& StaticConnection::getConnection()
{
    return conn;
}
