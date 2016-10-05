TEMPLATE = subdirs
SUBDIRS += QIcsMediaManager \
SimpleUiController \
SimpleDeviceManager \
FileSystemDevice

# if not building the Qt based players for Genivi platforms comment out the following lines
# !linux-oe-g++ {
    SUBDIRS += \
    QIcsAudioVideoPlayer \
    QIcsMediaPlayer \
# }
