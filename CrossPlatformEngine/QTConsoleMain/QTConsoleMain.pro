QT += core
QT -= gui

TARGET = QTConsoleMain
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    Console/CommandLine.cpp \
    Engine/base.cpp \
    Engine/app.cpp

HEADERS += \
    Console/CommandLine.h \
    Console/utils.h \
    Engine/base.h \
    Engine/app.h

