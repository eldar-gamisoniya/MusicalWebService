#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdio.h>
#include "RequestHandler.h"

void error_log(const char* msg)
{
   static std::ofstream error;

   if(!error.is_open())
   {
      error.open("/tmp/errlog", std::ios_base::out | std::ios_base::app);
      error.imbue(std::locale(error.getloc(), new boost::posix_time::time_facet()));
   }

   error << '[' << boost::posix_time::second_clock::local_time() << "] " << msg << std::endl;
}

int main()
{
   try
   {
      Fastcgipp::Manager<RequestHandler> fcgi;
      fcgi.handler();
   }
   catch(std::exception& e)
   {
      error_log(e.what());
   }
}
