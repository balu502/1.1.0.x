#-------------------------------------------------
#
# Project created by QtCreator 2019-04-16T09:57:43
#
#-------------------------------------------------
QT       += widgets
Qt       += gui
QT       += xml
QT       += network

TEMPLATE = lib
DEFINES += EMAIL_LIBRARY
CONFIG += dll

include (../../config.pro)

VERSION = 9.1.0.19
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += email.cpp

HEADERS += email.h\
        email_global.h

#--- Protocol ---
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
HEADERS  += protocol.h\
            utility.h
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}


#--- SMTP Lib ---
SMTP_LIBRARY_LOCATION = ../../../SMTP_Client/simple
INCLUDEPATH += $$SMTP_LIBRARY_LOCATION/src
DEPENDPATH += $$SMTP_LIBRARY_LOCATION/src

SMTPEMAIL = SMTPEmail
CONFIG(debug, debug|release) {SMTPEMAIL = $${SMTPEMAIL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${SMTPEMAIL}

#out
TARGET = $$qtLibraryTarget(Email)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/email_ru.ts\
                translations/email_en.ts

RESOURCES += \
    resource.qrc
