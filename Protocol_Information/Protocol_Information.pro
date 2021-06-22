#-------------------------------------------------
#
# Project created by QtCreator 2016-03-29T15:53:54
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui



TEMPLATE = lib
DEFINES += PROTOCOL_INFORMATION_LIBRARY
CONFIG += dll
include (../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#in

SOURCES += protocol_information.cpp

HEADERS += protocol_information.h\
        protocol_information_global.h

INCLUDEPATH += ../Analysis/Protocol
INCLUDEPATH += ../Analysis/Algorithm
INCLUDEPATH += ../Analysis/RT_Analysis

#--- QWT ---
QWT_LOCATION = ../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#out
TARGET = $$qtLibraryTarget(ProtInfo)
DLLDESTDIR = ../$${CURRENT_BUILD}
TRANSLATIONS = translations/ProtInfo_ru.ts\
                translations/ProtInfo_en.ts

RESOURCES += \
    resource.qrc


