#-------------------------------------------------
#
# Project created by QtCreator 2019-02-01T16:38:09
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += TEST_HRM_LIBRARY
CONFIG += dll
include (../../../config.pro)

VERSION = 9.1.0.17
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

INCLUDEPATH += ../../../Analysis/Protocol
INCLUDEPATH += ../../../Analysis/RT_Analysis
INCLUDEPATH += ../../test_editor

#in
SOURCES += test_hrm.cpp

HEADERS += test_hrm.h\
        test_hrm_global.h

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
TARGET = $$qtLibraryTarget(test_hrm)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/test_plugins
TRANSLATIONS = translations/test_hrm_ru.ts\
               translations/test_hrm_en.ts


RESOURCES += \
    resource.qrc
