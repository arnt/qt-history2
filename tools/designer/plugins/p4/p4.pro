TEMPLATE	= lib
CONFIG+= qt warn_on release plugin
HEADERS	= p4.h
SOURCES	= main.cpp p4.cpp
INTERFACES = diffdialog.ui submitdialog.ui
DESTDIR		= ../../../../plugins/designer
TARGET		= p4plugin

isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)

INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/designer
INSTALLS += target
