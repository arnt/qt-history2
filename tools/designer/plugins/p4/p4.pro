TEMPLATE	= lib
CONFIG		= qt warn_on release plugin
HEADERS	= p4.h
SOURCES	= main.cpp p4.cpp
INTERFACES = diffdialog.ui submitdialog.ui
DESTDIR		= $(QTDIR)/plugins
TARGET		= p4plugin
INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
