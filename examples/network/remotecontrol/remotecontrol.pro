GUID 		= {b002b20e-ca56-479d-87dd-8b51e5c45dd3}
TEMPLATE	= app
TARGET		= remotecontrol

QCONFIG         += network
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
