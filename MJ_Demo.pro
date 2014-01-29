#-------------------------------------------------
#
# Project created by QtCreator 2013-10-21T09:28:06
#
#-------------------------------------------------

QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MJ_Demo
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    FrameObserver.cpp \
    Vimba_Wrapper.cpp \
    Camera_Thread.cpp \
    win_qextserialport.cpp \
    qextserialport.cpp \
    qextserialbase.cpp

HEADERS  += mainwindow.h \
    FrameObserver.h \
    Vimba_Wrapper.h \
    Camera_Thread.h \
    win_qextserialport.h \
    qextserialport.h \
    qextserialbase.h

FORMS    += mainwindow.ui

INCLUDEPATH +=  "C:/Program Files/Allied Vision Technologies/AVTVimbaSDK_1.1" \
                "C:/opencv/build/include"

QMAKE_LIBDIR += "C:/opencv/build/x86/vc10/lib" \
                "C:/Program Files/Allied Vision Technologies/AVTVimbaSDK_1.1/VimbaCPP/Lib/Win32" \

LIBS += VimbaCPP.lib opencv_core246.lib opencv_imgproc246.lib opencv_highgui246.lib opencv_features2d246.lib opencv_nonfree246.lib opencv_flann246.lib

TRANSLATIONS	= MJ_Demo_zh_CN.ts
