#-------------------------------------------------
#
# Project created by QtCreator 2018-02-06T16:18:07
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui

TARGET = analyser_Quantity
TEMPLATE = lib
CONFIG += dll
DEFINES += ANALYSER_QUANTITY_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#IN
INCLUDEPATH += ../../RT_Analysis
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src

SOURCES += analyser_quantity.cpp

HEADERS += analyser_quantity.h\
        analyser_quantity_global.h

#--- QWT ---
QWT_LOCATION = ../../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#--- Protocol ---
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${PROTOCOL}

#...Algorithm...
ALGORITHM = algorithm
CONFIG(debug, debug|release) {ALGORITHM = $${ALGORITHM}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${ALGORITHM}

#--- GSL ---
GSL_LOCATION = ../../../../GSL/GSL-2.3-MinGW
INCLUDEPATH += $${GSL_LOCATION}/include/
LIBS += $${GSL_LOCATION}/bin/libgsl-23.dll
LIBS += $${GSL_LOCATION}/bin/libgslcblas-0.dll

#OUT
TARGET = $$qtLibraryTarget(analyser_quantity)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/analyser_plugins
TRANSLATIONS = translations/analyser_quantity_ru.ts\
                translations/analyser_quantity_en.ts

RESOURCES += \
    resource.qrc
