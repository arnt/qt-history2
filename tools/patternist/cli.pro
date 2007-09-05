TEMPLATE = app
LIBS += -lQtPatternist
TARGET = patternist
DESTDIR = $(QTDIR)/bin
QT -= gui
QT += xml
CONFIG -= app_bundle

# Note that ColorOutput.cpp and ColoringMessageHandler.cpp are also used internally
# in libQtPatternist. See src/api/api.pri.
SOURCES = main.cpp ColorOutput.cpp ColoringMessageHandler.cpp

HEADERS = main.h ColorOutput.h ColoringMessageHandler.h
include(../src/common.pri)
