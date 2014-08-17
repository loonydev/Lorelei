TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11 -fPIC

SOURCES += \
    lorelei.cpp

HEADERS += \
    lorelei.h \
    common.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix|win32: LIBS += -lsqlite3
