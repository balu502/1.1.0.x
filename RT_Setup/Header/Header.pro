#-------------------------------------------------
#
# Project created by QtCreator 2018-01-16T09:53:41
#
#-------------------------------------------------

QT       += widgets
QT       += core
QT       += gui
QT       += xml

TEMPLATE = lib
CONFIG += dll
DEFINES += HEADER_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.18
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis

SOURCES += header_test.cpp

HEADERS += header_test.h\
        header_global.h

#--- Protocol & Utility ---
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(header_test)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/header_test_ru.ts\
               translations/header_test_en.ts

RESOURCES += \
    resource.qrc
