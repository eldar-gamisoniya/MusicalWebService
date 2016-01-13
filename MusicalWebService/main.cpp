#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdio.h>
#include <fastcgi++/manager.hpp>
#include <fstream>
#include <stdio.h>
#include <thread>
#include "RequestHandler.h"

#define THREAD_COUNT 10

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

void managerFunction()
{
    while (true)
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
}

int main()
{
    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_COUNT; ++i)
        threads.push_back(std::move(std::thread(managerFunction)));

    for (int i = 0; i < THREAD_COUNT; ++i)
        threads.at(i).join();

    /*try
    {
       Fastcgipp::Manager<RequestHandler> fcgi;
       fcgi.handler();
    }
    catch(std::exception& e)
    {
       error_log(e.what());
    }*/
}
