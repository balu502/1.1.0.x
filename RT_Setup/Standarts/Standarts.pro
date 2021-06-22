#-------------------------------------------------
#
# Project created by QtCreator 2020-12-11T14:58:52
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui
QT       += xml

TEMPLATE = lib
DEFINES += STANDARTS_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.31
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#in
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis

SOURCES += standarts.cpp

HEADERS += standarts.h\
        standarts_global.h

#--- Protocol & Utility ---
#INCLUDEPATH += ../../Analysis/Protocol
#INCLUDEPATH += ../../Analysis/RT_Analysis
#INCLUDEPATH += ../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(Standarts)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/Standarts_ru.ts\
               translations/Standarts_en.ts

RESOURCES += \
    resource.qrc

