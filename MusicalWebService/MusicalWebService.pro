#-------------------------------------------------
#
# Project created by QtCreator 2015-12-23T20:04:50
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = MusicalWebService
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11

LIBS += \
 -lboost_system \
 -lboost_thread \
 -lfastcgipp


TEMPLATE = app


SOURCES += \
    main.cpp \
    RequestHandler.cpp \
    DatabaseHandler.cpp

HEADERS += \
    RequestHandler.h \
    DatabaseHandler.h \
    UserModel.h
