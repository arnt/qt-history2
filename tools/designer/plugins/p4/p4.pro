# Project ID used by some IDEs
GUID 		= {7e5dfbed-64f6-4c1a-bebe-cbdb12ea0911}
TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= p4.h
SOURCES		= main.cpp p4.cpp
FORMS		= diffdialog.ui submitdialog.ui
DESTDIR		= ../../../../plugins/designer
TARGET		= p4plugin

INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/interfaces


target.path += $$plugins.path/designer
INSTALLS 	+= target
