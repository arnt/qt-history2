TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= rc2ui.h
SOURCES		= main.cpp rc2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= rcplugin
INCLUDEPATH	+= ../../interfaces

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
