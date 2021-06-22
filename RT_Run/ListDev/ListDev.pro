#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T16:08:10
#
#-------------------------------------------------

QT       += widgets
QT       += network
Qt       += gui


TEMPLATE = lib
DEFINES += LISTDEV_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.13
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
#INCLUDEPATH += ../../Main_RT

SOURCES += listdev.cpp

HEADERS += listdev.h\
        listdev_global.h

#out
TARGET = $$qtLibraryTarget(ListDev)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/listdev_ru.ts\
                translations/listdev_en.ts

RESOURCES += \
    resource.qrc
