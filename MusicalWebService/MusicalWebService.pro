#-------------------------------------------------
#
# Project created by QtCreator 2015-12-23T20:04:50
#
#-------------------------------------------------

TARGET = MusicalWebService
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   -= qt
CONFIG   += c++11

LIBS += \
 -lboost_system \
 -lboost_thread \
 -lfastcgipp \
 -lmongocxx \
 -lbsoncxx

SOURCES += \
    main.cpp \
    RequestHandler.cpp \
    DatabaseHandler.cpp \
    StaticConnection.cpp

HEADERS += \
    RequestHandler.h \
    DatabaseHandler.h \
    UserModel.h \
    StaticConnection.h
