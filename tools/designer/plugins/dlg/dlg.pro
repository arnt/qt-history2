TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= dlg2ui.h
SOURCES		= main.cpp dlg2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= dlgplugin
INCLUDEPATH	+= ../../interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS	+= target
