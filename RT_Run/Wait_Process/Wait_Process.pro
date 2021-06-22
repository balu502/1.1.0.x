#-------------------------------------------------
#
# Project created by QtCreator 2016-07-11T11:55:21
#
#-------------------------------------------------

QT       += widgets
Qt       += gui

TEMPLATE = lib
DEFINES += WAIT_PROCESS_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#in
SOURCES += wait_process.cpp

HEADERS += wait_process.h\
        wait_process_global.h

#out
TARGET = $$qtLibraryTarget(WaitProcess)
DLLDESTDIR = ../../$${CURRENT_BUILD}

