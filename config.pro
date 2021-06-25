#-------------------------------------------------
#
# Project created by QtCreator 2011-10-28T09:54:36
#
#-------------------------------------------------

#DEFINES += CALIBRATION

#DEFINES += RT_NAME=\\\"$$RealTime\\\"
#DEFINES += CALIBR_NAME=\\\"$$Calibr\\\"

#contains(DEFINES, CALIBRATION){
#    Application_NAME = Calibr
#}else{
#    Application_NAME = RealTime
#}
#DEFINES += APP_NAME=\\\"$$Application_NAME\\\"


CONFIG += release
#CONFIG += debug

CONFIG(debug, debug|release) {CURRENT_BUILD = Run_project}
CONFIG(release, debug|release) {CURRENT_BUILD = Run_project_release}

CONFIG(debug, debug|release) {SUFFIX =  d}
CONFIG(release, debug|release) {SUFFIX =}


