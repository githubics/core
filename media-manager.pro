TEMPLATE = subdirs
SUBDIRS += QIcsMediaManager \
SimpleUiController \
SimpleDeviceManager \
FileSystemDevice

# not building the Qt based players for Genivi platforms
!linux-oe-g++ {
    SUBDIRS += \
    QIcsAudioVideoPlayer \
    QIcsMediaPlayer \
}
