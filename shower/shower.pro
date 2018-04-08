#-------------------------------------------------
#
# Project created by QtCreator 2018-04-06T22:57:20
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = shower
CONFIG   += console
CONFIG   -= app_bundle
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

TEMPLATE = app

QT += x11extras
LIBS += -lX11 -lXext

SOURCES += \
    ../vshow.cpp \
    ../shower.cpp \
    ../recver.cpp

HEADERS += \
    ../vshow.h \
    ../types.h \
    ../recver.h


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
