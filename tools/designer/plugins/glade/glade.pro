TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= glade2ui.h
SOURCES		= main.cpp glade2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= gladeplugin
INCLUDEPATH	+= ../../interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/designer
INSTALLS	+= target
