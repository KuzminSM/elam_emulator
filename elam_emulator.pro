#-------------------------------------------------
#
# Project created by QtCreator 2018-02-11T13:04:37
#
#-------------------------------------------------

QT += core gui serialport qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = elam_emulator
TEMPLATE = app

CONFIG += qscintilla2

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
        mainwindow.cpp \
    bitsmodel.cpp \
    hexitemdelegate.cpp \
    hexspinbox.cpp \
    registermodel.cpp \
    circle.c \
    crc16.c \
    modbus.c \
    addplcdialog.cpp \
    settingsdialog.cpp \
    mb_regs.cpp \
    scripteditor.cpp \
    editorsettingdialog.cpp

HEADERS += \
        mainwindow.h \
    bitsmodel.h \
    circle.h \
    crc16.h \
    hexitemdelegate.h \
    hexspinbox.h \
    mb_regs.h \
    modbus.h \
    registermodel.h \
    addplcdialog.h \
    settingsdialog.h \
    scripteditor.h \
    editorsettingdialog.h

FORMS += \
        mainwindow.ui \
    addplcdialog.ui \
    settingsdialog.ui \
    scripteditor.ui \
    scripteditor.ui \
    editorsettingdialog.ui

RESOURCES += \
    elam_emulator.qrc
