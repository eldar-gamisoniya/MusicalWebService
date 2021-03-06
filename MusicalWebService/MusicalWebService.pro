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
CONFIG   += lpthread

LIBS += \
 -lboost_system \
 -lboost_thread \
 -lfastcgipp \
 -lmongocxx \
 -lbsoncxx \
 -pthread

SOURCES += \
    main.cpp \
    RequestHandler.cpp \
    DatabaseHandler.cpp \
    StaticConnection.cpp \
    ReturnCodes.cpp

HEADERS += \
    RequestHandler.h \
    DatabaseHandler.h \
    UserModel.h \
    StaticConnection.h \
    ReturnCodes.h \
    AudioModel.h \
    PlaylistModel.h
