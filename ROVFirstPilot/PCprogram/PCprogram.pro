#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T13:23:12
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia multimediawidgets
QT       += network

LIBS += -L$$PWD/SDL-1.2.15/lib/ -lSDL.dll
INCLUDEPATH += $$PWD/SDL-1.2.15/include/SDL

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PCprogram
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dudp.cpp \
    worker.cpp

HEADERS  += mainwindow.h \
    dudp.h \
    worker.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
