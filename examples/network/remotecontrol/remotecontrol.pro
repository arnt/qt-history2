TEMPLATE	= app
TARGET		= remotecontrol

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config nocrosscompiler

HEADERS		= startup.h \
		  remotectrlimpl.h \
		  ipcserver.h
SOURCES		= main.cpp \
		  startup.cpp \
		  remotectrlimpl.cpp \
		  ipcserver.cpp
INTERFACES	= remotectrl.ui \
		  maindialog.ui
