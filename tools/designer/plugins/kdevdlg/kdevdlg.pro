GUID 		= {e40b066e-343b-47c6-a249-32b5c4d4e315}
TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= kdevdlg2ui.h
SOURCES		= main.cpp kdevdlg2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= kdevdlgplugin
INCLUDEPATH	+= ../../interfaces


target.path += $$plugins.path/designer
INSTALLS 	+= target
