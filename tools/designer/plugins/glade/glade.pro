TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= glade2ui.h
SOURCES		= main.cpp glade2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= gladeplugin
INCLUDEPATH	+= ../../interfaces

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
