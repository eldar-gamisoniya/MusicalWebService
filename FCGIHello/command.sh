g++ hello.cpp -o hello -lboost_system -lboost_thread -lfastcgipp
spawn-fcgi -p 8000 -n hello
