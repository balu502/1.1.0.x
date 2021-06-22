#-------------------------------------------------
#
# Project created by QtCreator 2012-01-20T10:50:14
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += dll
DEFINES += DIALOGS_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.28
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
#VERSION = 1.0.0
SOURCES += select_tubes.cpp

INCLUDEPATH += ../RT_Analysis
INCLUDEPATH += ../Algorithm
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../DockWidgetTitleBar

HEADERS += dialogs_global.h\
           select_tubes.h

#--- DockWidgetTitleBar ---
DOCKWIDGET = DockWidgetTitleBar
CONFIG(debug, debug|release) {DOCKWIDGET = $${DOCKWIDGET}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${DOCKWIDGET}

#--- QWT ---
QWT_LOCATION = ../../../QWT
INCLUDEPATH += $${QWT_LOCATION}/src
TRANSLATIONS = translations/select_ru.ts\
                translations/select_en.ts




RESOURCES += \
    resource.qrc



#out
TARGET = $$qtLibraryTarget(dialogs)
DLLDESTDIR = ../../$${CURRENT_BUILD}


