#-------------------------------------------------
#
# Project created by QtCreator 2017-12-01T09:49:44
#
#-------------------------------------------------

QT       += widgets xml
Qt       += gui

TEMPLATE = lib
DEFINES += ABOUTMODULS_LIBRARY
CONFIG += dll
CONFIG += skip_target_version_ext
include (../../config.pro)

VERSION = 1.1.0.22
QMAKE_TARGET_COPYRIGHT = dna-technology


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#in
SOURCES += aboutmoduls.cpp

HEADERS += aboutmoduls.h\
        aboutmoduls_global.h

LIBS +=libversion

#out
TARGET = $$qtLibraryTarget(about_moduls)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/aboutmoduls_ru.ts\
                translations/aboutmoduls_en.ts

RESOURCES += \
    resource.qrc
