TEMPLATE	= app
TARGET		= remotecontrol

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, full-config)"

HEADERS		= startup.h \
		  remotectrlimpl.h \
		  ipcserver.h
SOURCES		= main.cpp \
		  startup.cpp \
		  remotectrlimpl.cpp \
		  ipcserver.cpp
INTERFACES	= remotectrl.ui \
		  maindialog.ui
