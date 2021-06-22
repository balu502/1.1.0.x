#-------------------------------------------------
#
# Project created by QtCreator 2013-01-11T14:44:11
#
#-------------------------------------------------

QT       += widgets
QT       += xml
QT       += opengl

TEMPLATE = lib
DEFINES += GRAPH_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.34
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#QMAKE_LFLAGS += -enable-auto-import


#in
#VERSION = 1.0.1

SOURCES += graph.cpp

HEADERS += graph.h\
        Graph_global.h

INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
INCLUDEPATH += ../RT_Analysis

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

#out
TARGET = $$qtLibraryTarget(Graph)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/graph_ru.ts\
                translations/graph_en.ts

RESOURCES += \
    resources_graph.qrc
