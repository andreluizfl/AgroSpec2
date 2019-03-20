#-------------------------------------------------
#
# Project created by QtCreator 2018-04-25T17:03:10
#
#-------------------------------------------------

QT       += core gui
CONFIG += console c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = AgroSpec2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QCUSTOMPLOT_USE_LIBRARY
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

message("Begin")
QMAKE_CXXFLAGS+= -fpermissive
CXXFLAGS += -fpermissive

SOURCES += \
    main.cpp \
    oceanspectrometer.cpp \
    gui/testwindow.cpp \
    gui/groupboxspec.cpp \
    gui/dialog.cpp \
    module/modulemanager.cpp \
    module/spectrometermodule.cpp \
    interface/itest.cpp \
    interface/ievents.cpp \
    interface/ispectrometer.cpp \
    include/qcustomplot.cpp \
    listener.cpp \
    communication/ftdicomm.cpp \
    datamanipulation.cpp

HEADERS += \
    oceanspectrometer.h \
    gui/testwindow.h \
    gui/groupboxspec.h \
    gui/dialog.h \
    module/modulemanager.h \
    module/spectrometermodule.h \
    interface/itest.h \
    interface/ievents.h \
    interface/ispectrometer.h \
    include/qcustomplot.h \
    listener.h \
    communication/ftdicomm.h \
    communication/icommunication.h \
    datamanipulation.h

FORMS += \
    gui/testwindow.ui \
    gui/groupboxspec.ui \
    gui/dialog.ui

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/gui
INCLUDEPATH += $$PWD/include/gsl
INCLUDEPATH += $$PWD/include/seabreeze

LIBS += -L"$$PWD/lib"

win32 {
    message("Compiling for Windows")
    contains(QT_ARCH,i386):{
        message("Architecture 32 bits")
        LIBS += -L"$$PWD/lib/win32"
        LIBS += -lSeaBreeze -lusb -lusb-1.0 -lws2_32 -lconfuse -lftdi1

        CONFIG(debug, debug|release) {
            message("debug mode")
            OBJECTS_DIR = ".obj_debug"
            DESTDIR = $$PWD/bin/debug
        }

        CONFIG(release, debug|release) {
            message("release mode")
            OBJECTS_DIR = ".obj_release"
            DESTDIR = $$PWD/bin/release
        }
    }

    contains(QT_ARCH,x86_64):{
        message("Architecture 64 bits")
        LIBS += -L"$$PWD/lib/win64"
        #LIBS += -lSeaBreeze -lusb -lusb-1.0 -lws2_32 -lconfuse -lftdi1

        CONFIG(debug, debug|release) {
            message("debug mode")
            OBJECTS_DIR = ".obj_debug"
            DESTDIR = $$PWD/bin/debug
        }

        CONFIG(release, debug|release) {
            message("release mode")
            OBJECTS_DIR = ".obj_release"
            DESTDIR = $$PWD/bin/release
        }
    }
}

unix {
    message("Compiling for Linux")

    contains(QT_ARCH,i386):{
        message("Architecture 32 bits")
        LIBS += -L"/usr/lib/"
        LIBS += -L"$$PWD/lib/linux64"
    }

    contains(QT_ARCH,x86_64):{
        message("Architecture 64 bits")
        LIBS += -L"/usr/lib/"
        LIBS += -L"$$PWD/lib/linux64"
        LIBS += -lusb -lSeaBreeze -lusb-1.0 -lftdi1
    }

    contains(QT_ARCH,arm64):{
        message("Architecture arm64")
        LIBS += -L"/usr/lib/"
        LIBS += -L"/usr/lib/aarch64-linux-gnu/"
        LIBS += -L"$$PWD/lib/linuxaarch64"
        LIBS += -lSeaBreeze -lusb -lusb-1.0 -lftdi1
    }

    CONFIG(debug, debug|release) {
        message("debug mode")
        OBJECTS_DIR = ".obj_debug"
        DESTDIR = $$PWD/bin/debug
    }

    CONFIG(release, debug|release) {
        message("release mode")
        OBJECTS_DIR = ".obj_release"
        DESTDIR = $$PWD/bin/release
    }




}

#message (HOST:$$QMAKE_HOST.arch)
#RCC_DIR = "$$PWD\Build\RCCFiles"
#UI_DIR = "$$ParentDirectory\Build\UICFiles"
#MOC_DIR = "$$ParentDirectory\Build\MOCFiles"
#LIBS += -lgsl -lSeaBreeze -lusb -lusb-1.0 -lws2_32 -lconfuse -lftdi1 -lqcustomplot2

#DISTFILES += \    myapp.rc
#RESOURCES += \      Resources.qrc
#RC_FILE = myapp.rc

message("End")

RESOURCES += \
    Resources.qrc
