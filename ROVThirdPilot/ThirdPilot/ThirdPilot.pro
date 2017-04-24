
QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ThirdPilot
TEMPLATE = app


DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += main.cpp\
        thirdpilot.cpp \
    cameraview.cpp

HEADERS  += thirdpilot.h \
    cameraview.h

FORMS +=

RESOURCES += \
    res.qrc
