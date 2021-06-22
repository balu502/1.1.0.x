#-------------------------------------------------
#
# Project created by QtCreator 2017-07-05T14:38:51
#
#-------------------------------------------------

QT       += widgets xml
Qt       += gui
QT       += axcontainer

TEMPLATE = lib
DEFINES += TEST_EDITOR_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.28
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#in
INCLUDEPATH += ../../Analysis/Protocol
SOURCES += test_editor.cpp

HEADERS += test_editor.h\
        test_editor_global.h \
        test_interface.h

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/RT_Analysis
INCLUDEPATH += ../../Analysis/Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(test_editor)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/test_editor_ru.ts\
               translations/test_editor_en.ts

RESOURCES += \
    resource.qrc
