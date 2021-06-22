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
DEFINES += TEST_CALIBRATION_LIBRARY
CONFIG += dll
include (../../../config.pro)

VERSION = 9.1.0.8
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

INCLUDEPATH += ../../../Analysis/Protocol
INCLUDEPATH += ../../../Analysis/RT_Analysis
INCLUDEPATH += ../../test_editor

#in
SOURCES += test_calibration.cpp

HEADERS += test_calibration.h\
        test_calibration_global.h

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
TARGET = $$qtLibraryTarget(test_calibration)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/test_plugins
TRANSLATIONS = translations/test_calibration_ru.ts\
               translations/test_calibration_en.ts


RESOURCES += \
    resource.qrc
