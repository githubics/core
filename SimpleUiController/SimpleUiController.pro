#-------------------------------------------------
#
# Project created by QtCreator 2016-02-05T11:00:12
#
#-------------------------------------------------

QT       += core qml quick

TARGET = SimpleUiController
TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../plugins

SOURCES += \
    simpleuicontroller.cpp \
    playlistmodel.cpp \
    coverartprovider.cpp

HEADERS += \
    ../QIcsMediaManager/mediamanagercontrollerinterface.h \
    ../QIcsMediaManager/mediamanagertypes.h \
    simpleuicontroller.h \
    playlistmodel.h \
    coverartprovider.h

DISTFILES += \
    main.qml \
    MediaButton.qml \
    SimpleUiController.json \
    TimeLineSlider.qml \
    AudioElement.qml \
    PlaylistView.qml \
    VideoElement.qml \
    BlueToothElement.qml

RESOURCES += \
    qml.qrc

linux-oe-g++ {
    # Default rules for deployment.
    include(deployment.pri)
    target.path=/opt/QIcsMediaManager/plugins
    INTALLS+=target
}
