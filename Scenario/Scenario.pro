#-------------------------------------------------
#
# Project created by QtCreator 2019-01-18T11:39:55
#
#-------------------------------------------------

QT       += widgets
Qt       += gui
QT       += xml

TEMPLATE = lib
DEFINES += SCENARIO_LIBRARY
CONFIG += dll
include (../config.pro)

VERSION = 9.1.0.31
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += scenario.cpp

HEADERS += scenario.h\
        scenario_global.h \
        point_action.h

#--- Protocol & Utility ---
INCLUDEPATH += ../Analysis/Protocol
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${PROTOCOL}


#out
TARGET = $$qtLibraryTarget(Scenario)
DLLDESTDIR = ../$${CURRENT_BUILD}
TRANSLATIONS = translations/scenario_ru.ts\
                translations/scenario_en.ts

RESOURCES += \
    resource.qrc

