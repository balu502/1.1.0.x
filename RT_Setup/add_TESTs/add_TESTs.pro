#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T16:41:50
#
#-------------------------------------------------

QT       += widgets
Qt       += gui


TEMPLATE = lib
DEFINES += ADD_TESTS_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.29
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#in
INCLUDEPATH += ../../Analysis/Protocol

SOURCES += add_tests.cpp

HEADERS += add_tests.h\
        add_tests_global.h

#out
TARGET = $$qtLibraryTarget(add_TESTs)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/add_ru.ts\
               translations/add_en.ts

RESOURCES += \
    resource.qrc
