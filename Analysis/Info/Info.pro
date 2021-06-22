#-------------------------------------------------
#
# Project created by QtCreator 2017-07-03T11:59:07
#
#-------------------------------------------------

QT       += widgets
Qt       += gui

TEMPLATE = lib
DEFINES += INFO_LIBRARY
CONFIG += dll
include (../../config.pro)
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../RT_Analysis

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += info.cpp

HEADERS += info.h\
        info_global.h


#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src

#--- Protocol Information ---
INCLUDEPATH += ../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}

#out
TARGET = $$qtLibraryTarget(Info)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/info_ru.ts\
                translations/info_en.ts


RESOURCES += \
    resource.qrc

