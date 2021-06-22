#-------------------------------------------------
#
# Project created by QtCreator 2018-03-17T23:28:09
#
#-------------------------------------------------

QT      += core
QT      += widgets
QT      += gui
QT      += printsupport
QT      += xml
QT      += charts

TEMPLATE = lib
DEFINES += REPORT_PARAMETERIZATION_LIBRARY
CONFIG += dll
include (../../config.pro)
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../RT_Analysis

VERSION = 9.1.0.16
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


DEFINES += QT_DEPRECATED_WARNINGS

#in
SOURCES += report_parameterization.cpp

HEADERS += report_parameterization.h\
        report_parameterization_global.h

#--- Protocol ---
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
#HEADERS  += protocol.h\
#            utility.h
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- NCReport ---
NCREPORT_LOCATION = ../../../NCReport
INCLUDEPATH += $${NCREPORT_LOCATION}/include
NCREPORT = NCReport2
CONFIG(debug, debug|release) {NCREPORT = NCReportDebug2}
LIBS += -L$${NCREPORT_LOCATION}/lib -l$${NCREPORT}

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#out
TARGET = $$qtLibraryTarget(report_param)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/report_param_ru.ts\
                translations/report_param_en.ts

RESOURCES += \
    resource.qrc
