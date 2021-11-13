#-------------------------------------------------
#
# Project created by QtCreator 2018-08-01T16:41:58
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui

TARGET = analyser_Relative
TEMPLATE = lib
CONFIG += dll
DEFINES += ANALYSER_RELATIVE_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#IN
INCLUDEPATH += ../../RT_Analysis
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src

SOURCES += analyser_relative.cpp

HEADERS += analyser_relative.h\
        analyser_relative_global.h

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
TARGET = $$qtLibraryTarget(analyser_relative)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/analyser_plugins
TRANSLATIONS = translations/analyser_relative_ru.ts\
                translations/analyser_relative_en.ts

#RESOURCES += \
#    resource.qrc

RESOURCES += \
    resource.qrc
