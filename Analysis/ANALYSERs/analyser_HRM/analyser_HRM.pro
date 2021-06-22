#-------------------------------------------------
#
# Project created by QtCreator 2018-08-01T16:41:58
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui

TARGET = analyser_HRM
TEMPLATE = lib
CONFIG += dll
DEFINES += ANALYSER_HRM_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.17
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#IN
INCLUDEPATH += ../../RT_Analysis
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src

SOURCES += analyser_hrm.cpp \
    shapefactory.cpp

HEADERS += analyser_hrm.h\
        analyser_hrm_global.h \
    shapefactory.h

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
TARGET = $$qtLibraryTarget(analyser_hrm)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/analyser_plugins
TRANSLATIONS = translations/analyser_hrm_ru.ts\
                translations/analyser_hrm_en.ts

RESOURCES += \
    resource.qrc
