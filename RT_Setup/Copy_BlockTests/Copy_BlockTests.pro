#-------------------------------------------------
#
# Project created by QtCreator 2018-07-10T11:52:47
#
#-------------------------------------------------

QT       += widgets xml
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += COPY_BLOCKTESTS_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.23
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#in
INCLUDEPATH += ../../Analysis/Protocol
SOURCES += copy_blocktests.cpp

HEADERS += copy_blocktests.h\
        copy_blocktests_global.h

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis
INCLUDEPATH += ../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(Copy_BlockTests)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/Copy_BlockTests_ru.ts\
               translations/Copy_BlockTests_en.ts

RESOURCES += \
    resource.qrc
