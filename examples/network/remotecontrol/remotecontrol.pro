TEMPLATE	= app
TARGET		= remotecontrol

QT         += network compat
CONFIG		+= qt uic3 warn_on release

HEADERS		= startup.h \
		  remotectrlimpl.h \
		  ipcserver.h
SOURCES		= main.cpp \
		  startup.cpp \
		  remotectrlimpl.cpp \
		  ipcserver.cpp
INTERFACES	= remotectrl.ui \
		  maindialog.ui
