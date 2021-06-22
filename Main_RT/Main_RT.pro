#-------------------------------------------------
#
# Project created by QtCreator 2016-01-21T09:54:22
#
#-------------------------------------------------

QT       += core gui
QT       += qml quick script quickwidgets
QT       += network
QT       += xml
QT       += datavisualization
QT       += axcontainer
QT       += printsupport
QT       += websockets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (../config.pro)

#---------------------------------------------------
#DEFINES += CALIBRATION  # Calibration !!!!!!!!!!!!!!
#---------------------------------------------------

contains(DEFINES, CALIBRATION){
    Application_NAME = DTcheck
}else{
    Application_NAME = DTmaster
}
DEFINES += APP_NAME=\\\"$$Application_NAME\\\"

contains(DEFINES, CALIBRATION){
    TARGET = $${Application_NAME}
    VERSION = 1.1.0.14
}else{
    TARGET = $${Application_NAME}
    VERSION = 1.1.0.14
}
#----------------------------------------------------

TEMPLATE = app
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
QMAKE_TARGET_COPYRIGHT = dna-technology


#INCLUDEPATH += C:/OpenSSL-Win32/include

#CONFIG += console

#IN
SOURCES +=  main.cpp\
            main_rt.cpp \
    sp.cpp \
    preference.cpp \
    list_webprotocols.cpp \
    sp_rt.cpp \
    webdialog.cpp \
    wrapperax.cpp

HEADERS  += main_rt.h \
    sp.h \
    preference.h \
    list_webprotocols.h \
    sp_rt.h \
    webdialog.h \
    wrapperax.h

RESOURCES += resource.qrc

#DISTFILES += qml/web.qml
OTHER_FILES += qml/web.qml
OTHER_FILES += qml/integrator.qml

#LIBS += -lShell32
LIBS +=libversion

#--- About Moduls ---
INCLUDEPATH += ../Help/AboutModuls
ABOUT_MODULS = about_moduls
CONFIG(debug, debug|release) {ABOUT_MODULS = $${ABOUT_MODULS}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${ABOUT_MODULS}

#--- NCReport ---
NCREPORT_LOCATION = ../../NCReport
INCLUDEPATH += $${NCREPORT_LOCATION}/include
NCREPORT = NCReport2
CONFIG(debug, debug|release) {NCREPORT = NCReportDebug2}
LIBS += -L$${NCREPORT_LOCATION}/lib -l$${NCREPORT}

#--- Scenario ---
INCLUDEPATH += ../Scenario
SCENARIO = Scenario
CONFIG(debug, debug|release) {SCENARIO = $${SCENARIO}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${SCENARIO}

#--- HttpProcess ---
INCLUDEPATH += ../HttpProcess
HTTPPROCESS = HttpProcess
CONFIG(debug, debug|release) {HTTPPROCESS = $${HTTPPROCESS}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${HTTPPROCESS}

#--- Protocol ---
INCLUDEPATH += ../Analysis/Protocol
#INCLUDEPATH += ../Algorithm
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${PROTOCOL}

#--- Email ---
#INCLUDEPATH += ../Email
#EMAIL = Email
#CONFIG(debug, debug|release) {EMAIL = $${EMAIL}$${SUFFIX}}
#LIBS += -L../$${CURRENT_BUILD} -l$${EMAIL}

#--- Crypto ---
INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x86/include
#LIBS += -L../$${CURRENT_BUILD} -llibcrypto-1_1
#LIBS += -L../$${CURRENT_BUILD} -llibssl-1_1

#OUT
DESTDIR = ../$${CURRENT_BUILD}
TRANSLATIONS = translations/ru.ts \
               translations/en.ts \
               translations/ch.ts
RC_ICONS = $$PWD/DTm.ico
#RC_ICONS = $$PWD/RT.ico

DISTFILES += \
    qml/integrator.qml \
    qml/balu_dir.qml \
    qml/main-rtapp.qml \
    qml/main-consumer.qml




