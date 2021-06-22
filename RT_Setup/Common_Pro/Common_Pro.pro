#-------------------------------------------------
#
# Project created by QtCreator 2017-11-15T12:04:10
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui
QT       += xml

TEMPLATE = lib
DEFINES += COMMON_PRO_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis

SOURCES += common_pro.cpp

HEADERS += common_pro.h\
        common_pro_global.h

#out
TARGET = $$qtLibraryTarget(Common_Pro)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/Common_Pro_ru.ts\
               translations/Common_Pro_en.ts

RESOURCES += \
    resource.qrc

