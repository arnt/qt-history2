TEMPLATE	= app
CONFIG		= xbase
DEFINES		+= XBASE_DEBUG
include(../include/shared.pri)
SOURCES	        += main.cpp
LIBPATH		+= ../lib
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb

# install
target.path=$$QDB_INSTALL_BINPATH
isEmpty(target.path):target.path=/usr/local/bin
INSTALLS += target



