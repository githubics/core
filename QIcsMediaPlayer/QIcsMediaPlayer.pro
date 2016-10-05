#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T20:45:35
#
#-------------------------------------------------

QT       += core gui multimedia

TARGET = QIcsMediaPlayer
TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../plugins

SOURCES += qicsmediaplayer.cpp

HEADERS += qicsmediaplayer.h \
../QIcsMediaManager/mediaplayerinterface.h

DISTFILES += QIcsMediaPlayer.json

linux-oe-g++ {
    # Default rules for deployment.
    include(deployment.pri)
    target.path=/opt/QIcsMediaManager/plugins
    INSTALLS+=target
}
