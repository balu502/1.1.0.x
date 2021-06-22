#-------------------------------------------------
#
# Project created by QtCreator 2016-01-22T18:10:47
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += network
QT       += xml
QT       += axcontainer
QT       += printsupport
QT       += opengl

TEMPLATE = lib
DEFINES += ANALYSIS_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.34
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#DEFINES += VERS=VERSION
DEFINES += VERS=\\\"9.1.0.6\\\"
#VERSION = 9.1.0.3
#DEFINES += DLL_VERSION=\\\"$$VERSION\\\"


#in
INCLUDEPATH += ../../Scenario

SOURCES +=  analysis.cpp

HEADERS +=  analysis.h\
            analysis_global.h\
            alg_interface_qevent.h \
            alg_interface.h \
            define_PCR.h \
    generic_interface.h \
    analysis_interface.h

RESOURCES += \
            resource.qrc

LIBS +=libversion

#--- Protocol ---
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
HEADERS  += protocol.h\
            utility.h
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- Graph ---
INCLUDEPATH += ../Graph
GRAPH = Graph
CONFIG(debug, debug|release) {GRAPH = $${GRAPH}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${GRAPH}

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src

#--- dialogs ---
INCLUDEPATH += ../dialogs
DIALOGS = dialogs
CONFIG(debug, debug|release) {DIALOGS = $${DIALOGS}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${DIALOGS}

#--- DockWidgetTitleBar ---
INCLUDEPATH += ../DockWidgetTitleBar
DOCKWIDGET = DockWidgetTitleBar
CONFIG(debug, debug|release) {DOCKWIDGET = $${DOCKWIDGET}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${DOCKWIDGET}

#--- Protocol Information ---
INCLUDEPATH += ../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}

#--- Information ---
INCLUDEPATH += ../Info
PROT_INFO = Info
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}

#--- Test Editor ---
INCLUDEPATH += ../../RT_Setup/test_editor
TEST_EDITOR = test_editor
CONFIG(debug, debug|release) {TEST_EDITOR = $${TEST_EDITOR}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${TEST_EDITOR}

#--- Common_Pro ---
INCLUDEPATH += ../../RT_Setup/Common_Pro
COMMON_PRO = Common_Pro
CONFIG(debug, debug|release) {COMMON_PRO = $${COMMON_PRO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${COMMON_PRO}

#--- NCReport ---
NCREPORT_LOCATION = ../../../NCReport
INCLUDEPATH += $${NCREPORT_LOCATION}/include
#NCREPORT = NCReport2
#CONFIG(debug, debug|release) {NCREPORT = NCReportDebug2}
#LIBS += -L$${NCREPORT_LOCATION}/lib -l$${NCREPORT}

#--- Report_Parameterization ---
INCLUDEPATH += ../Report_Parameterization
REPORT_PARAM = report_param
CONFIG(debug, debug|release) {REPORT_PARAM = $${REPORT_PARAM}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${REPORT_PARAM}

#--- Report ---
INCLUDEPATH += ../Reports/Report
REPORT = report
CONFIG(debug, debug|release) {REPORT = $${REPORT}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${REPORT}

#--- Video Archive ---
INCLUDEPATH += ../Video_Archive
VIDEO_ARCHIVE = Video_Archive
CONFIG(debug, debug|release) {VIDEO_ARCHIVE = $${VIDEO_ARCHIVE}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${VIDEO_ARCHIVE}

#--- Email ---
INCLUDEPATH += ../Email
INCLUDEPATH += ../../../SMTP_Client/simple/src
EMAIL = Email
CONFIG(debug, debug|release) {EMAIL = $${EMAIL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${EMAIL}


#OUT
TARGET = $$qtLibraryTarget(analysis_rt)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/analysis_ru.ts\
                translations/analysis_en.ts
