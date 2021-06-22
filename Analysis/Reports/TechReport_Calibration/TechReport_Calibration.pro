#-------------------------------------------------
#
# Project created by QtCreator 2019-01-09T15:49:45
#
#-------------------------------------------------

QT       += core xml
QT       += widgets
Qt       += gui
QT       += printsupport

TARGET = TechReport_Calibration
TEMPLATE = lib
CONFIG += dll
DEFINES += TECHREPORT_CALIBRATION_LIBRARY
include (../../../config.pro)

VERSION = 9.1.0.40
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#IN
INCLUDEPATH += ../Report
INCLUDEPATH += ../../Protocol
INCLUDEPATH += ../../Algorithm
INCLUDEPATH += ../../Algorithm/src

SOURCES += techreport_calibration.cpp

HEADERS += techreport_calibration.h\
        techreport_calibration_global.h

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
TARGET = $$qtLibraryTarget(techreport_calibration)
DLLDESTDIR = ../../../$${CURRENT_BUILD}/ControlDepartment
TRANSLATIONS = translations/techreport_calibration_ru.ts\
                translations/techreport_calibration_en.ts


RESOURCES += \
    resource.qrc

