#-------------------------------------------------
#
# Project created by QtCreator 2015-12-23T11:00:36
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotorManager
TEMPLATE = app
RC_FILE = app.rc


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    assets/assets.qrc
