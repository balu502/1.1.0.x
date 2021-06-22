#-------------------------------------------------
#
# Project created by QtCreator 
#
#-------------------------------------------------

QT       += widgets
QT       += xml

TEMPLATE = lib
DEFINES += GRAPH_FLUOR_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.25
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in

SOURCES += graph_fluor.cpp

HEADERS += graph_fluor.h\
        Graph_Fluor_global.h

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/Algorithm
INCLUDEPATH += ../../Analysis/Algorithm/src
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}


#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#out
TARGET = $$qtLibraryTarget(GraphFluor)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/graph_fluor_ru.ts\
                translations/graph_fluor_en.ts

RESOURCES += \
    resources_graph.qrc
