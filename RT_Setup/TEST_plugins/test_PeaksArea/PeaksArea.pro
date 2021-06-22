#-------------------------------------------------
#
# Project created by QtCreator 2017-12-31T14:53:34
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += PEAKSAREA_LIBRARY
CONFIG += dll
include (../../../config.pro)

VERSION = 9.1.0.18
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

INCLUDEPATH += ../../../Analysis/Protocol
INCLUDEPATH += ../../../Analysis/RT_Analysis

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#in
INCLUDEPATH += ../../test_editor

SOURCES += peaksarea.cpp
HEADERS += peaksarea.h\
        peaksarea_global.h

#--- Protocol & Utility ---
INCLUDEPATH += ../../../Analysis/Protocol
INCLUDEPATH += ../../../Analysis/RT_Analysis
INCLUDEPATH += ../../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- QWT ---
QWT_LOCATION = ../../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src

#--- Protocol Information ---
INCLUDEPATH += ../../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${PROT_INFO}

#--- Common ---
INCLUDEPATH += ../../Common
COMMON_PRO = Common
CONFIG(debug, debug|release) {COMMON_PRO = $${COMMON_PRO}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${COMMON_PRO}

#--- Header ---
INCLUDEPATH += ../../Header
HEADER_TEST = header_test
CONFIG(debug, debug|release) {HEADER_TEST = $${HEADER_TEST}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${HEADER_TEST}

#out
TARGET = $$qtLibraryTarget(test_peaksarea)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/test_plugins
TRANSLATIONS = translations/peaksarea_ru.ts\
                translations/peaksarea_en.ts

RESOURCES += \
    resource.qrc

