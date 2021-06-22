#-------------------------------------------------
#
# Project created by QtCreator 2020-11-10T15:41:33
#
#-------------------------------------------------

QT       += widgets
Qt       += gui


TEMPLATE = lib
DEFINES += CONTRACT_LIBRARY
CONFIG += dll
include (../../config.pro)

VERSION = 9.1.0.29
QMAKE_TARGET_COPYRIGHT = dna-technology
CONFIG += skip_target_version_ext


#in
SOURCES += contract.cpp

HEADERS += contract.h\
        contract_global.h

#out
TARGET = $$qtLibraryTarget(Contract)
DLLDESTDIR = ../../$${CURRENT_BUILD}
TRANSLATIONS = translations/contract_ru.ts\
                translations/contract_en.ts

RESOURCES += \
    resource.qrc

