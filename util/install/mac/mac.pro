TARGET = unpack
TEMPLATE = app
SOURCES = main.cpp
INCLUDEPATH += ../archive
CONFIG += qt 
CONFIG -= resource_fork
unix:LIBS += -L$(QTDIR)/util/install/archive -larq

