#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <fstream>
#include <string>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <stdlib.h>

class RequestHandler: public Fastcgipp::Request<char>
{
public:
   virtual bool response();

   void setError(std::string error);
   void setHttpHeaders(std::string code);

   std::string getId();

   bool addUser();
   bool authorizeUser();
   bool getUser();
};

#endif
