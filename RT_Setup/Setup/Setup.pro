#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T20:55:52
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += network
QT       += xml
QT       += axcontainer

TEMPLATE = lib
DEFINES += SETUP_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.28
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#---------------------
DEFINES += TIS
#---------------------

#in
SOURCES += setup.cpp \
    zoomplate.cpp

HEADERS += setup.h\
        setup_global.h \
        zoomplate.h

#--- sp.h ---
INCLUDEPATH += ../../Main_RT

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src


#--- Protocol Information ---
INCLUDEPATH += ../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/RT_Analysis
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- Add_TESTs ---
INCLUDEPATH += ../add_TESTs
ADD_TESTS = add_TESTs
CONFIG(debug, debug|release) {ADD_TESTS = $${ADD_TESTS}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${ADD_TESTS}

#--- Test Editor ---
INCLUDEPATH += ../test_editor
TEST_EDITOR = test_editor
CONFIG(debug, debug|release) {TEST_EDITOR = $${TEST_EDITOR}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${TEST_EDITOR}

#--- Common_Pro ---
INCLUDEPATH += ../Common_Pro
COMMON_PRO = Common_Pro
CONFIG(debug, debug|release) {COMMON_PRO = $${COMMON_PRO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${COMMON_PRO}

#--- Copy_BlockTests
INCLUDEPATH += ../Copy_BlockTests
COPY_BLOCKTESTS = Copy_BlockTests
CONFIG(debug, debug|release) {COPY_BLOCKTESTS = $${COPY_BLOCKTESTS}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${COPY_BLOCKTESTS}

#OUT
TARGET = $$qtLibraryTarget(setup_rt)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/setup_ru.ts\
               translations/setup_en.ts


RESOURCES += \
    resource.qrc

