QT       += widgets
Qt       += gui
QT       += xml

TEMPLATE = lib
DEFINES += RDML_LIBRARY
CONFIG += dll
CONFIG += c++11

include (../../config.pro)

VERSION = 1.0.0.1
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#in
SOURCES += \
    rdml.cpp

HEADERS += \
    RDML_global.h \
    rdml.h

#--- Protocol ---
INCLUDEPATH += ../Protocol
INCLUDEPATH += ../Algorithm
HEADERS  += protocol.h\
            utility.h
PROTOCOL = protocol
CONFIG(debug, debug|release) {PROTOCOL = $${PROTOCOL}$${SUFFIX}}
LIBS += -L../../$${CURRENT_BUILD} -l$${PROTOCOL}

#out
TARGET = $$qtLibraryTarget(rdml)
DLLDESTDIR = ../../$${CURRENT_BUILD}

