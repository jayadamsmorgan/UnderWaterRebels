QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TherdPilot
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += main.cpp\
        therdpilot.cpp \
    paintscene.cpp

HEADERS  += therdpilot.h \
    paintscene.h

FORMS    += therdpilot.ui

QMAKE_CXXFLAGS += -std=gnu++14

RESOURCES += \
    iop.qrc
