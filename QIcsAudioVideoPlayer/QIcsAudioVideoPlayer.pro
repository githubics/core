#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T20:45:35
#
#-------------------------------------------------

QT       += core gui quick multimedia qml

TARGET = QIcsAudioVideoPlayer
TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../plugins

SOURCES += \
    mediaplayercontroller.cpp \
    qicsmediaplayer.cpp

HEADERS += \
../QIcsMediaManager/mediaplayerinterface.h \
    mediaplayercontroller.h \
    qicsmediaplayer.h

RESOURCES += qml.qrc


DISTFILES += \
    QIcsAudioVideoPlayer.json

linux-oe-g++ {
    # Default rules for deployment.
    include(deployment.pri)
    target.path=/opt/QIcsMediaManager/plugins
    INSTALLS+=target
}
