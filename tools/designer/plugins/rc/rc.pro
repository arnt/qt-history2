GUID 		= {28715368-f1ad-40e2-b62a-0e7e412b6c88}
TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= rc2ui.h
SOURCES		= main.cpp rc2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= rcplugin
INCLUDEPATH	+= ../../interfaces


target.path += $$plugins.path/designer
INSTALLS 	+= target
