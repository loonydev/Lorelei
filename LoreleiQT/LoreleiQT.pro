#-------------------------------------------------
#
# Project created by QtCreator 2014-08-05T12:53:15
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LoreleiQT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-Lorelei-Desktop-Debug/release/ -lLorelei
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-Lorelei-Desktop-Debug/debug/ -lLorelei
else:unix: LIBS += -L$$PWD/../build-Lorelei-Desktop-Debug/ -lLorelei

INCLUDEPATH += $$PWD/../Lorelei
DEPENDPATH += $$PWD/../Lorelei
