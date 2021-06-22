#-------------------------------------------------
#
# Project created by QtCreator 2017-12-11T11:09:55
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui


TEMPLATE = lib
DEFINES += RUNTIME_LIBRARY
CONFIG += dll
CONFIG += skip_target_version_ext
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology

DEFINES += QT_DEPRECATED_WARNINGS

#in
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis

SOURCES += runtime.cpp

HEADERS += runtime.h\
        runtime_global.h

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src

#--- Protocol Information ---
INCLUDEPATH += ../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}


#out
TARGET = $$qtLibraryTarget(run_time)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/runtime_ru.ts\
                translations/runtime_en.ts

RESOURCES += \
    resource.qrc

