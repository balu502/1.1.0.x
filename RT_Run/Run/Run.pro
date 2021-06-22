#-------------------------------------------------
#
# Project created by QtCreator 2016-01-29T16:30:46
#
#-------------------------------------------------

QT       += widgets
QT       += network
QT       += core
QT       += xml
QT       += axcontainer
#QT       += multimedia


TEMPLATE = lib
DEFINES += RUN_LIBRARY
include (../../config.pro)

DEFINES += VERS=\\\"9.1.0.7\\\"
VERSION = 9.1.0.39
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
INCLUDEPATH += ../../Analysis/dialogs
INCLUDEPATH += ../../Analysis/DockWidgetTitleBar
INCLUDEPATH += ../../Scenario

INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x86/include
LIBS += -L../../$${CURRENT_BUILD} -llibcrypto-1_1
LIBS += -L../../$${CURRENT_BUILD} -llibssl-1_1

#LIBS += -lssl -lcrypto
#LIBS += -LC:/OpenSSL-Win32_old/lib/MinGW -llibeay32
#LIBS += -LC:/OpenSSL-Win32_old/lib/MinGW -lssleay32
#LIBS += -L../../$${CURRENT_BUILD} -llibssl32
#LIBS += -L../../$${CURRENT_BUILD} -llibeay32
#LIBS += -L../../$${CURRENT_BUILD} -lssleay32

SOURCES += run.cpp \
    worker_wait.cpp \
    scandialog.cpp


HEADERS += run.h\
        run_global.h\
        request_dev.h \
        worker_wait.h \
    device_param.h \
    scandialog.h


INCLUDEPATH += ../../Analysis/RT_Analysis

#--- Graph ---
INCLUDEPATH += ../Graph_fluor
INCLUDEPATH += ../Graph_temperature
GRAPH_FLUOR = GraphFluor
GRAPH_TEMPERATURE = GraphTemperature
CONFIG(debug, debug|release) {GRAPH_FLUOR = $${GRAPH_FLUOR}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD}/pages/run -l$${GRAPH_FLUOR}
CONFIG(debug, debug|release) {GRAPH_TEMPERATURE = $${GRAPH_TEMPERATURE}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD}/pages/run -l$${GRAPH_TEMPERATURE}

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src

#--- ListDev ---
INCLUDEPATH += ../ListDev
LIST_DEV = ListDev
CONFIG(debug, debug|release) {LIST_DEV = $${LIST_DEV}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${LIST_DEV}

#--- Protocol Information ---
INCLUDEPATH += ../../Protocol_Information
PROT_INFO = ProtInfo
CONFIG(debug, debug|release) {PROT_INFO = $${PROT_INFO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROT_INFO}

#--- Protocol & Utility ---
INCLUDEPATH += ../../Analysis/Protocol
INCLUDEPATH += ../../Analysis/Algorithm
INCLUDEPATH += ../../Analysis/Algorithm/src
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- dialogs ---
DIALOGS = dialogs
CONFIG(debug, debug|release) {DIALOGS = $${DIALOGS}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${DIALOGS}

#--- DockWidgetTitleBar ---
DOCKWIDGET = DockWidgetTitleBar
CONFIG(debug, debug|release) {DOCKWIDGET = $${DOCKWIDGET}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${DOCKWIDGET}

#--- Wait_Process ---
INCLUDEPATH += ../Wait_Process
WAIT_PROCESS = WaitProcess
CONFIG(debug, debug|release) {WAIT_PROCESS = $${WAIT_PROCESS}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${WAIT_PROCESS}

#--- VideoImage ---
INCLUDEPATH += ../Video
#INCLUDEPATH += ../Plate_3D
VIDEO = video_rt
CONFIG(debug, debug|release) {VIDEO = $${VIDEO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${VIDEO}
#PLATE_3D = Plate_3D
#CONFIG(debug, debug|release) {PLATE_3D = $${PLATE_3D}$${SUFFIX}}
#LIBS += -L../../$${CURRENT_BUILD} -l$${PLATE_3D}

#--- Expo ---
INCLUDEPATH += ../Expo
EXPO = expo_rt
CONFIG(debug, debug|release) {EXPO = $${EXPO}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${EXPO}

#--- RunTime ---
INCLUDEPATH += ../RunTime
RUNTIME = run_time
CONFIG(debug, debug|release) {RUNTIME = $${RUNTIME}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${RUNTIME}

#--- Contract ---
INCLUDEPATH += ../Contract
CONTRACT = Contract
CONFIG(debug, debug|release) {CONTRACT = $${CONTRACT}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${CONTRACT}

#--- Crypto ---
#INCLUDEPATH += ../Crypto
#CRYPTO = Crypto
#CONFIG(debug, debug|release) {CRYPTO = $${CRYPTO}$${SUFFIX}}
#LIBS += -L../../$${CURRENT_BUILD} -l$${CRYPTO}


#OUT
TARGET = $$qtLibraryTarget(run_rt)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/run_ru.ts\
                translations/run_en.ts

RESOURCES += \
    resource.qrc

DISTFILES +=
