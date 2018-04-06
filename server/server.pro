#-------------------------------------------------
#
# Project created by QtCreator 2018-04-06T22:48:44
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QT += x11extras
LIBS += -lX11 -lXext

SOURCES += \
    ../vcompress.cpp \
    ../server.cpp \
    ../sender.cpp \
    ../capture.cpp \
    ../vshow.cpp

HEADERS += \
    ../vcompress.h \
    ../types.h \
    ../sender.h \
    ../capture.h \
    ../vshow.h


unix:!macx:!symbian: LIBS += -L$$PWD/../build/lib/ -lavcodec

INCLUDEPATH += $$PWD/../build/include
DEPENDPATH += $$PWD/../build/include

unix:!macx:!symbian: LIBS += -L$$PWD/../build/lib/ -lavformat

INCLUDEPATH += $$PWD/../build/include
DEPENDPATH += $$PWD/../build/include

unix:!macx:!symbian: LIBS += -L$$PWD/../build/lib/ -lavutil

INCLUDEPATH += $$PWD/../build/include
DEPENDPATH += $$PWD/../build/include

unix:!macx:!symbian: LIBS += -L$$PWD/../build/lib/ -lswscale

INCLUDEPATH += $$PWD/../build/include
DEPENDPATH += $$PWD/../build/include

unix:!macx:!symbian: LIBS += -L$$PWD/../build/lib/ -lx264

INCLUDEPATH += $$PWD/../build/include
DEPENDPATH += $$PWD/../build/include
