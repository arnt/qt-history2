TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= dlg2ui.h
SOURCES		= main.cpp dlg2ui.cpp
DESTDIR		= ../../../../plugins/designer
DEFINES 	+= QT_INTERNAL_XML
include( ../../../../src/qt_professional.pri )
TARGET		= dlgplugin
INCLUDEPATH	+= ../../interfaces

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
