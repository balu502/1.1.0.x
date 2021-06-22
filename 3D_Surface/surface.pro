
TEMPLATE = app

CONFIG += release

SOURCES += main.cpp \
           surfacegraph.cpp

HEADERS += surfacegraph.h

QT       += core gui
QT       += qml quick script quickwidgets
QT       += datavisualization
QT       += widgets

RESOURCES += surface.qrc \
    resource.qrc


OTHER_FILES += doc/src/* \
               doc/images/*

TRANSLATIONS = translations/3D_ru.ts\
                translations/3D_en.ts
