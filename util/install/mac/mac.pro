TARGET = unpackage
TEMPLATE = app
HEADERS = unpackdlgimpl.h
SOURCES = main.cpp unpackdlgimpl.cpp
INTERFACES += unpackdlg.ui
INCLUDEPATH += ../archive
CONFIG += qt
CONFIG -= resource_fork
unix:LIBS += -L$(QTDIR)/util/install/archive -larq

