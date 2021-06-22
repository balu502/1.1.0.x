#-------------------------------------------------
#
# Project created by QtCreator 2017-01-11T09:51:02
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += xml


TEMPLATE = lib
DEFINES += VIDEO_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.29
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
INCLUDEPATH += ../Run

SOURCES += video.cpp \
    zoom.cpp

HEADERS += video.h\
        video_global.h \
    zoom.h

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/Algorithm
INCLUDEPATH += ../../Analysis/Algorithm/src
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}


#out
TARGET = $$qtLibraryTarget(video_rt)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/video_ru.ts\
                translations/video_en.ts

RESOURCES += \
    resource.qrc


