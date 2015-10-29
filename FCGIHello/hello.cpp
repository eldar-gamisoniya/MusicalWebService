#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <stdio.h>
#include <string>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;
using namespace Fastcgipp;
using namespace boost;

void error_log(const char* msg)
{
   static ofstream error;
   if(!error.is_open())
   {
      error.open("/tmp/errlog", ios_base::out | ios_base::app);
      error.imbue(locale(error.getloc(), new posix_time::time_facet()));
   }

   error << '[' << posix_time::second_clock::local_time() << "] " << msg << endl;
}

class HelloWorld: public Request<wchar_t>
{

   bool response()
   {
      GenericDocument<UTF16<> > d;
      //Document d;

      d.SetObject();

      Document::AllocatorType& allocator = d.GetAllocator();

      wstring str = L"world";

      d.AddMember(L"hello", StringRef(str.c_str()), allocator);
      d.AddMember(L"uri", StringRef(environment().requestUri.c_str()), allocator);
      d.AddMember(L"method", environment().requestMethod, allocator);

      StringBuffer sb;
      //PrettyWriter<StringBuffer> writer(sb);
      PrettyWriter< StringBuffer, UTF16<> >  writerUTF16(sb);
      //d.Accept(writer);
      d.Accept(writerUTF16);

      //writerUTF16.StartObject();

      //writer.String("uri:");
      //writer.String("123");
      //wchar_t c[] = L"c";
      //writerUTF16.String();
      //writerUTF16.String(c);

      //writer.EndObject();

      out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
      out << sb.GetString();

      err << "Hello apache error log";

      return true;
   }

};

int main()
{
   try
   {
      Manager<HelloWorld> fcgi;
      fcgi.handler();
   }
   catch(std::exception& e)
   {
      error_log(e.what());
   }
}
