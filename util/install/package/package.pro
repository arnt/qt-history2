TEMPLATE = app
HEADERS = generatordlgimpl.h
SOURCES = main.cpp generatordlgimpl.cpp
INCLUDEPATH += ../archive $(QTDIR)/include ../keygen
INTERFACES = generatordlg.ui
TARGET  = package
CONFIG += qt
unix:LIBS += -L$(QTDIR)/util/install/archive -larq

