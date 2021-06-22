#-------------------------------------------------
#
# Project created by QtCreator 2016-01-25T15:53:00
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
DEFINES += SAVER_LIBRARY
include (../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += saver.cpp

HEADERS += saver.h\
        saver_global.h

#OUT
TARGET = $$qtLibraryTarget(saver_rt)
DLLDESTDIR = ../$${CURRENT_BUILD}/pages
TRANSLATIONS = translations/saver_ru.ts\
                translations/saver_en.ts

RESOURCES += \
    resource.qrc
