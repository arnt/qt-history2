QTDIR_build:REQUIRES        = network full-config nocrosscompiler
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= startup.h \
		  remotectrlimpl.h \
		  ipcserver.h

SOURCES		= main.cpp \
		  startup.cpp \
		  remotectrlimpl.cpp \
		  ipcserver.cpp

INTERFACES	= remotectrl.ui \
		  maindialog.ui

TARGET		= remotecontrol
