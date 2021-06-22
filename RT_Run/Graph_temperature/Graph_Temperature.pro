#-------------------------------------------------
#
# Project created by QtCreator 
#
#-------------------------------------------------

QT       += widgets
QT       += xml

TEMPLATE = lib
DEFINES += GRAPH_TEMPERATURE_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in

SOURCES += graph_temperature.cpp

HEADERS += graph_temperature.h\
        Graph_Temperature_global.h

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#out
TARGET = $$qtLibraryTarget(GraphTemperature)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/graph_temperature_ru.ts\
                translations/graph_temperature_en.ts

RESOURCES += \
    resources_graph.qrc
