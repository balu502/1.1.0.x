#-------------------------------------------------
#
# Project created by QtCreator 2017-10-31T10:26:06
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui
QT       += xml

TEMPLATE = lib
DEFINES += COMMON_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.8
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis

SOURCES += common.cpp

HEADERS += common.h\
        common_global.h

#--- Protocol & Utility ---
#INCLUDEPATH += ../../Analysis/Protocol
#INCLUDEPATH += ../../Analysis/RT_Analysis
#INCLUDEPATH += ../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(Common)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/Common_ru.ts\
               translations/Common_en.ts

RESOURCES += \
    resource.qrc

