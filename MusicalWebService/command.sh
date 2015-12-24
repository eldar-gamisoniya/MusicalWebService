g++ -std=c++0x RequestHandler.h -o RequestHandler -lboost_system -lboost_thread -lfastcgipp
g++ -std=c++0x RequestHandler.o main.cpp -o main -lboost_system -lboost_thread -lfastcgipp
spawn-fcgi -p 8120 -n main

