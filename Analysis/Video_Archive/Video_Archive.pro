#-------------------------------------------------
#
# Project created by QtCreator 2019-01-11T11:28:21
#
#-------------------------------------------------

QT      += core
QT      += widgets
QT      += gui


TEMPLATE = lib
DEFINES += VIDEO_ARCHIVE_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#in
SOURCES += video_archive.cpp

HEADERS += video_archive.h\
           video_archive_global.h

RESOURCES += \
    resource.qrc

#out
TARGET = $$qtLibraryTarget(Video_Archive)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/archive_ru.ts\
                translations/archive_en.ts

