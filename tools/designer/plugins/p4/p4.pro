TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= p4.h
SOURCES		= main.cpp p4.cpp
FORMS		= diffdialog.ui submitdialog.ui
DESTDIR		= ../../../../plugins/designer
TARGET		= p4plugin

isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)

INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/interfaces

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
