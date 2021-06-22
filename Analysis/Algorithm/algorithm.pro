#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T18:42:13
#
#-------------------------------------------------

QT       -= gui

TEMPLATE = lib
CONFIG += dll
DEFINES += ALGORITHM_LIBRARY
include (../../config.pro)

VERSION = 9.1.0.7
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext

#QMAKE_LFLAGS += -enable-auto-import

#in
#VERSION = 3.4.0
SOURCES +=  algorithm.cpp\
#            src/*.cpp \
            model_sigmoidfit.cpp

SOURCES += $$files(src/*.cpp, true)

HEADERS +=  algorithm.h\
            algorithm_global.h\
#            src/*.h \
            point_takeoff.h
HEADERS += $$files(src/*.h, true)


INCLUDEPATH += src

#--- GSL ---
GSL_LOCATION = ../../../GSL/GSL-2.3-MinGW
INCLUDEPATH += $${GSL_LOCATION}/include/
LIBS += $${GSL_LOCATION}/bin/libgsl-23.dll
LIBS += $${GSL_LOCATION}/bin/libgslcblas-0.dll



#out
TARGET = $$qtLibraryTarget(algorithm)
DLLDESTDIR = ../../$${CURRENT_BUILD}



