#-------------------------------------------------
#
# Project created by QtCreator 2019-11-19T17:12:45
#
#-------------------------------------------------

QT       += widgets network
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += HTTPPROCESS_LIBRARY
CONFIG += dll
include (../config.pro)

VERSION = 9.1.0.28
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

DEFINES += QT_DEPRECATED_WARNINGS

#in
SOURCES += httpprocess.cpp \
           http.cpp

HEADERS += httpprocess.h\
        httpprocess_global.h \
        http.h

#out
TARGET = $$qtLibraryTarget(HttpProcess)
DLLDESTDIR = ../$${CURRENT_BUILD}
TRANSLATIONS = translations/httpprocess_ru.ts\
                translations/httpprocess_en.ts

RESOURCES += \
    resource.qrc
