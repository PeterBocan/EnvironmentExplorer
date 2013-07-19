#
# This is a part of EnvironmentExplorer program
# which is licensed under LGPLv2.
#
# Github: https://github.com/PeterBocan/EnvironmentExplorer
# Author: https://twitter.com/PeterBocan
#

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
message($$CONFIG)

TARGET = EnvironmentExplorer
TEMPLATE = app

SOURCES += main.cpp \
           MainDialog.cpp \
           VariablesManager.cpp \
    MainDialogUi.cpp

HEADERS += MainDialog.h \
           VariablesManager.h \
           MainDialogUi.h

LIBS += -ladvapi32

RESOURCES += \
    resources.qrc

# Use external as a file...
win32-g++ {
    CONFIG -= embed_manifest_exe
    QMAKE_CXXFLAGS += -std=c++0x
}

win32-msvc2012 {
    # require administrator privileges to run in order to save variables...
    CONFIG += embed_manifest_exe
    QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'
}

