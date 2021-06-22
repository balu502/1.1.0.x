#-------------------------------------------------
#
# Project created by QtCreator 2019-02-01T18:12:42
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui

TARGET = analyser_Quality
TEMPLATE = lib
CONFIG += dll
DEFINES += ANALYSER_CALIBRATION_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.40
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#IN
INCLUDEPATH += ../../RT_Analysis
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src
INCLUDEPATH += ../../../../SMTP_Client/src
INCLUDEPATH += ../../../RT_Run/Run

SOURCES += analyser_calibration.cpp \
           total_results.cpp \
           mask.cpp \
           melt_curve.cpp


HEADERS += analyser_calibration.h\
           analyser_calibration_global.h \
           total_results.h \
           mask.h \
           melt_curve.h

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

#--- QWT ---
QWT_LOCATION = ../../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}



#OUT
TARGET = $$qtLibraryTarget(analyser_calibration)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/analyser_plugins
TRANSLATIONS = translations/analyser_calibration_ru.ts\
                translations/analyser_calibration_en.ts

RESOURCES += \
    resource.qrc

