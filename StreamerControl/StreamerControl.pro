#-------------------------------------------------
#
# Project created by QtCreator 2025-05-26T16:29:17
#
#-------------------------------------------------

QT       += core gui

QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS -= -Wunused-parameter
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS -= -Wunused-parameter
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_CXXFLAGS_WARN_ON -= -Wunused-parameter
QMAKE_CXXFLAGS_WARN_OFF += -Wunused-parameter
QMAKE_CXXFLAGS_WARN_OFF -= -Wno-unused-parameter
QMAKE_CXXFLAGS+= -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StreamerControl
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        cmainwindow.cpp \
    system/cmutex.cpp \
    system/cthread.cpp \
    system/system.cpp \
    cstreamer.cpp \
    ccommand.cpp

HEADERS += \
        cmainwindow.h \
    system/cmutex.h \
    system/cthread.h \
    system/system.h \
    cstreamer.h \
    ccommand.h

FORMS += \
        cmainwindow.ui
