#-------------------------------------------------
#
# Project created by QtCreator 2012-12-13T14:32:19
#
#-------------------------------------------------

QT       += gui
QT       += widgets

TEMPLATE = lib
CONFIG += dll
DEFINES += DOCKWIDGETTITLEBAR_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.8
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#in
#VERSION = 1.0.0
SOURCES += dockwidgettitlebar.cpp

HEADERS += dockwidgettitlebar.h\
           DockWidgetTitleBar_global.h

RESOURCES += \
    resource.qrc

#out
TARGET = $$qtLibraryTarget(DockWidgetTitleBar)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/dock_ru.ts\
                translations/dock_en.ts


