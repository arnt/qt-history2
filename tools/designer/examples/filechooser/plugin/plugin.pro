# Project ID used by some IDEs
GUID 	 = {99d9c1cc-2b8b-4783-9bb4-f97a0743eb96}
TEMPLATE = lib
LANGUAGE = C++
TARGET   = filechooser

SOURCES  += plugin.cpp ../widget/filechooser.cpp
HEADERS  += plugin.h ../widget/filechooser.h
DESTDIR   = ../../../../../plugins/designer

target.path=$$plugins.path

INSTALLS    += target
CONFIG      += qt warn_on release plugin
INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces
DBFILE       = plugin.db
