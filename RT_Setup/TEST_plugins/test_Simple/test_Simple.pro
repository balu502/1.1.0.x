#-------------------------------------------------
#
# Project created by QtCreator 2017-10-09T10:52:42
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += TEST_SIMPLE_LIBRARY
CONFIG += dll
include (../../../config.pro)

VERSION = 9.1.0.8
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

INCLUDEPATH += ../../../Analysis/Protocol
INCLUDEPATH += ../../../Analysis/RT_Analysis

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#in
INCLUDEPATH += ../../test_editor
INCLUDEPATH += ../../../Analysis/Protocol
SOURCES += test_simple.cpp

HEADERS += test_simple.h\
        test_simple_global.h

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
TARGET = $$qtLibraryTarget(test_simple)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/test_plugins
TRANSLATIONS = translations/test_simple_ru.ts\
                translations/test_simple_en.ts

RESOURCES += \
    resource.qrc


