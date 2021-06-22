#-------------------------------------------------
#
# Project created by QtCreator 2015-06-10T12:18:33
#
#-------------------------------------------------

QT       += xml
QT       += gui
#QT       += core gui

TARGET = Protocol
TEMPLATE = lib
CONFIG += dll
DEFINES += PROTOCOL_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.30

QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#IN
SOURCES += protocol.cpp\
           utility.cpp

HEADERS += protocol.h\
           utility.h

INCLUDEPATH += ../RT_Analysis

#...Algorithm...
INCLUDEPATH += ../Algorithm\
               ../Algorithm/src

ALGORITHM = algorithm
CONFIG(debug, debug|release) {ALGORITHM = $${ALGORITHM}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${ALGORITHM}


#OUT
TARGET = $$qtLibraryTarget(protocol)
DLLDESTDIR = ../../$${CURRENT_BUILD}
