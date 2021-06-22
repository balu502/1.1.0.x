#-------------------------------------------------
#
# Project created by QtCreator 2017-02-03T10:47:31
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += xml

TEMPLATE = lib
DEFINES += EXPO_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#in
INCLUDEPATH += ../Run

SOURCES += expo.cpp

HEADERS += expo.h\
        expo_global.h

#out
TARGET = $$qtLibraryTarget(expo_rt)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/expo_ru.ts\
                translations/expo_en.ts

RESOURCES += \
    resource.qrc
