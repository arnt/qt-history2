TARGET = unpackage
TEMPLATE = app
HEADERS = unpackdlgimpl.h licensedlgimpl.h
SOURCES = main.cpp unpackdlgimpl.cpp licensedlgimpl.cpp
INTERFACES += unpackdlg.ui licensedlg.ui
INCLUDEPATH += ../archive ../keygen
CONFIG += qt
unix:LIBS += -L$(QTDIR)/util/install/archive -larq

