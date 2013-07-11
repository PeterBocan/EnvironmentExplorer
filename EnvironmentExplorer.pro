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

RESOURCES += \
    resources.qrc
