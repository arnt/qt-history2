TEMPLATE = app
HEADERS = generatordlgimpl.h
SOURCES = main.cpp generatordlgimpl.cpp
INCLUDEPATH += ../archive
INTERFACES = generatordlg.ui
TARGET  = package
CONFIG += qt
unix:LIBS += -L../archive -larq

