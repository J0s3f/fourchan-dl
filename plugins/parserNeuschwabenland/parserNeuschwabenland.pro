#-------------------------------------------------
#
# Project created by QtCreator 2011-09-21T19:19:51
#
#-------------------------------------------------

QT       -= gui

win32:TARGET = ../../parserNeuschwabenland
else:TARGET = ../parserNeuschwabenland
TEMPLATE = lib
CONFIG += dll plugin

DEFINES += NEUSCHWABENLANDPARSER_LIBRARY _LIB_VERSION="\\\"0.9\\\""

SOURCES += parserNeuschwabenland.cpp

HEADERS += parserNeuschwabenland.h
