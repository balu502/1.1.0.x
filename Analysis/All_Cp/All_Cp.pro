#-------------------------------------------------
#
# Project created by QtCreator 2015-06-18T15:42:26
#
#-------------------------------------------------

QT       += widgets
QT       += xml
QT       += core
QT       += axcontainer


TARGET = All_Cp
TEMPLATE = lib
CONFIG += dll
DEFINES += ALL_CP_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.34
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#IN
INCLUDEPATH += ../RT_Analysis
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
INCLUDEPATH += ../Algorithm/src

SOURCES += all_cp.cpp \
           pcranalysis.cpp \
    crossbox.cpp

HEADERS += all_cp.h\
           all_cp_global.h \
           pcranalysis.h \
    crossbox.h

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#--- Protocol ---
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#...Algorithm...
ALGORITHM = algorithm
CONFIG(debug, debug|release) {ALGORITHM = $${ALGORITHM}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${ALGORITHM}

#OUT
TARGET = $$qtLibraryTarget(all_cp)
DLLDESTDIR = ../../$${CURRENT_BUILD}/plugins
TRANSLATIONS = translations/allcp_ru.ts\
                translations/allcp_en.ts

RESOURCES += \
    resource_cpall.qrc
