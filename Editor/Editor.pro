#-------------------------------------------------
#
# Project created by QtCreator 2016-12-06T15:57:05
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += network
QT       += xml


TEMPLATE = lib
DEFINES += EDITOR_LIBRARY
include (../config.pro)

VERSION = 9.1.0.33
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += editor.cpp \
           gradient.cpp \
    incr_temperature.cpp

HEADERS += editor.h\
        editor_global.h \
        gradient.h \
    incr_temperature.h

#--- QWT ---
QWT_LOCATION = ../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
QWT = qwt
VER__MAJ = 6
CONFIG(debug, debug|release) {QWT = $${QWT}$${SUFFIX}}
LIBS += -L$${QWT_LOCATION}/lib -l$${QWT}

#...Algorithm...
INCLUDEPATH += ../Analysis/Algorithm\
               ../Analysis/Algorithm/src

ALGORITHM = algorithm
CONFIG(debug, debug|release) {ALGORITHM = $${ALGORITHM}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${ALGORITHM}

#--- Protocol & Utility ---
INCLUDEPATH += ../Analysis/Protocol
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../$${CURRENT_BUILD} -l$${PROTOCOL}


#OUT
TARGET = $$qtLibraryTarget(editor_rt)
DLLDESTDIR = ../$${CURRENT_BUILD}
TRANSLATIONS = translations/editor_ru.ts\
                translations/editor_en.ts

RESOURCES += \
    resource.qrc
