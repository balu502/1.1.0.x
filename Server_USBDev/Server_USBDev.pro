#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T15:59:10
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (../config.pro)

#TARGET = Server_USBDev
TEMPLATE = app

#CONFIG += console


#in
LIBS += -lsetupapi \
        -lwinmm
#INCLUDEPATH += ../Cypress/USB DevStudio/Driver/Inc

SOURCES +=  main.cpp\
            server_usb.cpp\
            CypressUsb.cpp

HEADERS +=  server_usb.h\
            CypressUsb.h

#OUT
DESTDIR = ../$${CURRENT_BUILD}
TARGET = $$qtLibraryTarget(Server_USBDev)
TRANSLATIONS = translations/serverUSBDev_ru.ts\
               translations/serverUSBDev_en.ts

RESOURCES += \
    resource.qrc
