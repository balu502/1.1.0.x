#-------------------------------------------------
#
# Project created by QtCreator 2019-01-09T15:49:45
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui
QT       += printsupport

TARGET = Report_Relative
TEMPLATE = lib
CONFIG += dll
DEFINES += REPORT_RELATIVE_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#IN
INCLUDEPATH += ../Report
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src

SOURCES += report_relative.cpp


HEADERS += report_relative.h\
        report_relative_global.h

#--- Protocol ---
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${PROTOCOL}

#...Algorithm...
ALGORITHM = algorithm
CONFIG(debug, debug|release) {ALGORITHM = $${ALGORITHM}$${SUFFIX}}
LIBS += -L../../../$${CURRENT_BUILD} -l$${ALGORITHM}

#--- NCReport ---
NCREPORT_LOCATION = ../../../../NCReport
INCLUDEPATH += $${NCREPORT_LOCATION}/include
NCREPORT = NCReport2
CONFIG(debug, debug|release) {NCREPORT = NCReportDebug2}
LIBS += -L$${NCREPORT_LOCATION}/lib -l$${NCREPORT}

#OUT
TARGET = $$qtLibraryTarget(report_relative)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/report_plugins
TRANSLATIONS = translations/report_relative_ru.ts\
               translations/report_relative_en.ts


#RESOURCES += \
#    resource.qrc

RESOURCES += \
    resource.qrc

