TEMPLATE = app
HEADERS = generatordlgimpl.h qarchive.h
SOURCES = main.cpp generatordlgimpl.cpp qarchive.cpp
INTERFACES = generatordlg.ui
INCLUDEPATH += $(QTDIR)\src\3rdparty
TARGET  = generate
CONFIG += qt